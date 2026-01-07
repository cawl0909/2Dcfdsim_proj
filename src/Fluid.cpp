
#include <vector>
#include <cmath>
#include <iostream>
#include <random>

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