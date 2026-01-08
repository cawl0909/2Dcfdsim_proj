
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

double Fluid::grid_interpolation(double x, double y, std::string& field)
{

    //For me to remember
    // This would be a basic 2d bilinear interpolation but there needs to be some extra logic because the 
    // u,v and smoke fields are offset by different amounts to the typical cartesian grid points UGH!!! but MAC grids make life so much easier
    // I guess I'll slog through it 



    double one_over_cs = 1/cell_size;
    double half_cs = cell_size/2;

    double x_offset = 0.0;
    double y_offset = 0.0;

    int x_trans = std::max(std::min(x,numX*cell_size),cell_size);
    int y_trans = std::max(std::min(y,numY*cell_size),cell_size);

    std::vector<std::vector<double>> sample_field;

    if(field == "u")
    {
        sample_field = u_grid;
        y_offset = half_cs;
    }
    else if (field == "v")
    {
        sample_field = v_grid;
        x_offset = half_cs;
    }
    else if(field == "s")
    {
        sample_field =  mass;
        x_offset = half_cs;
        y_offset = half_cs;
    }

    int xg0 = std::min(std::floor(((x_trans-x_offset)/(cell_size))),(numX-1.0)); //bl x grid pos
    int xg1 = std::min(xg0+1,numX-1); //br x grid pos

    int yg0 = std::min(std::floor((y_trans-y_offset)/(cell_size)),numY-1.0);
    int yg1 = std::min(yg0+1,numY-1);
    


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