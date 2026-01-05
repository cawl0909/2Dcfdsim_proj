
#include <vector>
#include <cmath>
#include <iostream>
#include <random>

#include "Fluid.h"

Fluid::Fluid(double _density, int _numX, int _numY, double _h, double _over_relaxtion)
{
    fluid_density = _density;
    numX = _numX +2;
    numY = _numY +2;
    numCells = numX*numY;
    cell_size = _h;
    over_relaxation = _over_relaxtion;
    num = _numX*_numY;

    u_grid = std::vector<double>(numCells,0);
    v_grid = std::vector<double>(numCells,0);
    new_u_grid = std::vector<double>(numCells,0);
    new_v_grid = std::vector<double>(numCells,0);
    pressure = std::vector<double>(numCells,0);
    solid = std::vector<double>(numCells,0);
    mass = std::vector<double>(numCells,0);
    new_mass.resize(numCells,1.0);

}

void Fluid::integrate(double dt, double gravity)
{
    int n = numY;
    for(int i = 1; i < numX-1;i++)
    {
        for(int j = 1; j< numY-1;j++)
        {
            if ((solid[i*n + j] != 1.0) && (solid[i*n + j-1] != 1.0))
            {
                v_grid[i*n+j] += gravity*dt;
            }
        }
    }
}

void Fluid::solveIncompressability(int numIterations, double dt)
{
    int n = numY;
    double const_params = fluid_density*(dt*cell_size);

    for (int iter = 0; iter < numIterations; iter++)
    {
        for (int i = 1; i < numX-1; i++)
        {
            for (int j = 1; j < numY-1; j++)
            {
                if(solid[i*n +j] == 0.0)
                {
                    continue;
                }

                double s_left = solid[(i-1)*n +j];
                double s_right = solid[(i+1)*n + j];
                double s_top = solid[(i*n + j + 1)];
                double s_bottom = solid[(i*n + j -1)];

                double s_total = s_left + s_bottom + s_top + s_right;

                if(s_total == 0.0)
                {
                    continue;
                }

                double divergence_x = u_grid[(i+1)*n + j] - u_grid[i*n+j];
                double divergence_y = v_grid[i*n + j +1] - v_grid[i*n+j];
                double divergence = divergence_x + divergence_y;

                double temp = -(divergence/s_total);
                double temp = temp*over_relaxation;
                pressure[i*n+j] += const_params*temp;

                u_grid[i*n+j] -= (s_left*temp);
                u_grid[(i+1)+j] += (s_right*temp);
                v_grid[i*n+j] -= (s_bottom*temp);
                v_grid[i*n+j+1] += (s_top*temp);

            }
        }
    } 
}