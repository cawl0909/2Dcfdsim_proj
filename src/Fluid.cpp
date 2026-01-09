
#include <vector>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <cmath>

#include "Fluid.h"

Fluid::Fluid(double _density, int _numX, int _numY, double _h, double _over_relaxtion)
{
    fluid_density = _density;
    i_numX = _numX;
    i_numY = _numY;
    numX = _numX +2;
    numY = _numY +2; //ghost size delta added, these values will be passed
    numCells = numX*numY;
    cell_size = _h;
    over_relaxation = _over_relaxtion;
    num = _numX*_numY;

    u_grid = std::vector<std::vector<double>>(numX,std::vector<double>(numY,0));
    v_grid = std::vector<std::vector<double>>(numX,std::vector<double>(numY,0));
    new_u_grid = std::vector<std::vector<double>>(numX,std::vector<double>(numY,0));
    new_v_grid = std::vector<std::vector<double>>(numX,std::vector<double>(numY,0));
    pressure = std::vector<std::vector<double>>(numX,std::vector<double>(numY,0));
    solid = std::vector<std::vector<double>>(numX,std::vector<double>(numY,1.0));
    mass = std::vector<std::vector<double>>(numX,std::vector<double>(numY,0));
    new_mass = std::vector<std::vector<double>>(numX,std::vector<double>(numY,0));

}

double Fluid::get_divergence(int x, int y)
{
    double u_left = u_grid[x][y];
    double u_right = u_grid[x+1][y];

    double v_bottom = v_grid[x][y];
    double v_top = v_grid[x][y+1];

    return ((u_right-u_left) + (v_top - v_bottom));
}

void Fluid::integrate(double dt, double gravity)
{
    for(int i = 1; i < numX - 1; i++)
    {
        for(int j = 1; j < numY - 1; j++)
        {
            if((solid[i][j] != 0.0) && (solid[i][j-1] != 0.0))
            {
                v_grid[i][j] += gravity*dt;
            }
        }
    }
}

void Fluid::solveIncompressability(int numIterations, double dt)
{
    double const_param = (fluid_density*cell_size)/dt;

    for(int iter = 0; iter<numIterations;iter++)
    {
        for(int i = 1; i<numX-1;i++)
        {
            for(int j = 1; j < numY;j++)
            {
                if(solid[i][j] == 0.0){continue;}

                double s_left = solid[i-1][j];
                double s_right = solid[i+1][j];
                double s_bottom = solid[i-1][j];
                double s_top = solid[i+1][j];

                double s_total = s_left + s_right + s_bottom + s_top;

                if(s_total == 0.0){continue;}

                double div = get_divergence(i,j);

                double temp_p = (-div)/s_total;
                temp_p = temp_p * over_relaxation; 

                pressure[i][j] = pressure[i][j] + temp_p*(const_param);

                u_grid[i][j] = u_grid[i][j] - s_left*temp_p;
                u_grid[i+1][j] = u_grid[i+1][j] + s_top*temp_p;

                v_grid[i][j] = v_grid[i][j] - s_bottom*temp_p;
                v_grid[i][j+1] = v_grid[i][j+1] + s_top*temp_p;

            }
        }
    }
}

void Fluid::border_velocity_extrapolate() 
{
    for(int i = 0; i<numX;i++)
    {
        u_grid[i][0] =  u_grid[i][1]; // bottom ghost objects get the velocities from the proper cell just above
        u_grid[i][numY-1] = u_grid[i][numY-2];
    }
    for(int j = 0; j<numY;j++)
    {
        v_grid[0][j] = v_grid[1][j];
        v_grid[numX-1][j] = v_grid[numX-2][j];
    }
}

double Fluid::grid_interpolation(double x, double y, std::string field)
{

    //For me to remember
    // This would be a basic 2d bilinear interpolation but there needs to be some extra logic because the 
    // u,v and smoke fields are offset by different amounts to the typical cartesian grid points UGH!!! but MAC grids make life so much easier
    // I guess I'll slog through it 



    double cs_1 = 1.0/cell_size;
    double cs_2 = cell_size*0.5;

    double x_offset = 0.0;
    double y_offset = 0.0;

    double xi = std::max(std::min(x,numX*cell_size),cell_size);
    double yi = std::max(std::min(y,numY*cell_size),cell_size);

    std::vector<std::vector<double>> sample_field;

    if(field == "u")
    {
        sample_field = u_grid;
        y_offset = cs_2;
    }
    else if (field == "v")
    {
        sample_field = v_grid;
        x_offset = cs_2;
    }
    else if(field == "s")
    {
        sample_field =  mass;
        x_offset = cs_2;
        y_offset = cs_2;
    }

    int x0 = std::min(std::floor((xi-x_offset)*cs_1),numX-1.0);
    double tx = ((xi-x_offset)-x0*cell_size)*cs_1;
    int x1 = std::min(x0+1,numX-1); //br x grid pos

    int y0 = std::min(std::floor((yi-y_offset)*cs_1),numY-1.0);
    double ty = ((yi-y_offset)- y0*cell_size)*cs_1;
    int y1 = std::min(y0+1,numY-1);
    
    double sx = 1.0-tx;
    double sy = 1.0-ty;

    double interpolation = sx*sy*(sample_field[x0][y0]) + tx*sy*(sample_field[x1][y0]) + tx*ty*(sample_field[x1][y1]) + sx*ty*(sample_field[x0][y1]);

    return interpolation;
}

double Fluid::get_avg_u(int x, int y)
{
    double total = u_grid[x][y] + u_grid[x-1][y] + u_grid[x+1][y-1] + u_grid[x+1][y];
    return (total/4);
}

double Fluid::get_avg_v(int x, int y)
{
    double total = v_grid[x][y] + v_grid[x-1][y] + v_grid[x-1][y+1] + v_grid[x][y+1];
    return (total/4);
}

void Fluid::advect_velocity(double dt)
{
    new_u_grid = u_grid;
    new_v_grid = v_grid;

    double c2 = cell_size/2;

    for(int i = 1; i < numX-1;i++)
    {
        for(int j = 1; j< numY-1;j++)
        {
            if((solid[i][j] != 0) && (solid[i-1][j] != 0) && (j < numY-1))
            {
                double sp_x = i*cell_size;    //sim pos not grid pos
                double sp_y = j*cell_size + c2;

                double u = u_grid[i][j];
                double v = get_avg_v(i,j);

                sp_x = sp_x - (dt*u);
                sp_y = sp_y - (dt*v);

                u = grid_interpolation(sp_x,sp_y,"u");
                new_u_grid[i][j] = u;
            }

            if((solid[i][j] != 0) && (solid[i][j-1] != 0) &&(i<numX-1))
            {
                double sp_x = i*cell_size + c2;
                double sp_y = j*cell_size;

                double u = get_avg_v(i,j);
                double v = v_grid[i][j];

                sp_x = sp_x - (dt*u);
                sp_y = sp_y - (dt*v);

                v = grid_interpolation(sp_x,sp_y,"v");
                new_v_grid[i][j] = v;
            }
        }
    }

    u_grid = new_u_grid;
    v_grid = new_v_grid;
}

void Fluid::reset_pressure()
{
    for(int i = 0; i<numX;i++)
    {
        for(int j = 0; j<numY;j++)
        {
            pressure[i][j] = 0.0;
        }
    }
}

// -------------------------------------------------------------------------

void Fluid::randomise_velocities(std::mt19937& generator)
{
    std::uniform_real_distribution<double> dist(0,50.0);
    for(int i=0;i<numX;i++)
    {
        for(int j=0;j<numY;j++)
        {
            u_grid[i][j] = dist(generator);
        }
    }
    for(int i=0;i<numX;i++)
    {
        for(int j=0;j<numY;j++)
        {
            v_grid[i][j] = dist(generator);
        }
    }
}

// ------------------------------------------------------------------------

void Fluid::simulate(double dt, double grav, double num_iterations)
{
    integrate(dt,grav);

    reset_pressure();
    solveIncompressability(num_iterations,dt);
    border_velocity_extrapolate();
    advect_velocity(dt);
}