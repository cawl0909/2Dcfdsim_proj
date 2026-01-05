
#include <vector>
#include <cmath>
#include <iostream>
#include <random>

#include "Fluid.h"


Fluid::Fluid(size_t _grid_size_x, size_t _grid_size_y, double _cell_length ,double _time_step)
{
    init_gird_x = _grid_size_x;
    init_grid_y = _grid_size_y;
    grid_size_x = _grid_size_x+2; //extra boundary cell modification
    grid_size_y = _grid_size_y+2;
    cell_length = _cell_length;
    time_step = _time_step;
    velocity_grid_u =  std::vector<std::vector<double>>(grid_size_x+1,std::vector<double>(grid_size_y,0));
    velocity_grid_v =  std::vector<std::vector<double>>(grid_size_x,std::vector<double>(grid_size_y+1,0));

    pressure_grid =  std::vector<std::vector<double>>(grid_size_x,std::vector<double>(grid_size_y,0));
    smoke_field =  std::vector<std::vector<double>>(grid_size_x,std::vector<double>(grid_size_y,0));

    obstacle_grid =  std::vector<std::vector<int>>(grid_size_x,std::vector<int>(grid_size_y,0)); 
}

// ---------------------------------------------------------------------------------

void Fluid::apply_gravity()
{
    for(int i = 1; i<velocity_grid_v.size()-1;i++)
    {
        for(int j=1;j<velocity_grid_v[i].size()-1;j++)
        {
            if((obstacle_grid[i][j] != 0) && (obstacle_grid[i][j-1] != 0))
            {
                double new_v = velocity_grid_v[i][j] + G*time_step;
                velocity_grid_v[i][j] = new_v;
            }
        }
    }
}

void Fluid::pressure_solve(int interations)
{
    double overelxation = 1.7;
    double units_scale =  (fluid_density*cell_length)/time_step;

    for(int iteration = 0; iteration < interations; iteration++)
    {
        for(int i = 1; i<grid_size_x-1;i++)
        {
            for(int j=1; j<grid_size_y-1;j++)
            {
                if(is_solid(i,j))
                {
                    continue;
                }
                
                int sx_back = obstacle_grid[i-1][j];
                int sx_forward = obstacle_grid[i+1][j];
                int sy_up = obstacle_grid[i][j+1];
                int sy_down = obstacle_grid[i][j-1];
                
                int s = sx_back + sx_forward + sy_down + sy_up;
                if(s == 0)
                {
                    continue;
                }

                double div = (velocity_grid_u[i+1][j] - velocity_grid_u[i][j]) + (velocity_grid_v[i][j+1] - velocity_grid_v[i][j]);
                
                double p = -(div/s)*1.7; //overrelxation factor for gauss sidel

                pressure_grid[i][j] = pressure_grid[i][j] + p*units_scale;

                velocity_grid_u[i][j] -= sx_back*p;
                velocity_grid_u[i+1][j] += sx_forward*p;
                velocity_grid_v[i][j] -= sy_down*p;
                velocity_grid_v[i][j+1] += sy_up*p;
            }
        }
    }
}







// ---------------------------------------------------------------------------------

double Fluid::calculate_velocity_divergence(int x, int y)
{
    double velocity_u_left = velocity_grid_u[x][y];
    double velocity_u_right  = velocity_grid_u[x+1][y];

    double velocity_v_top = velocity_grid_v[x][y+1];
    double velocity_v_bottom = velocity_grid_v[x][y];

    double partial_u_x = (velocity_u_right-velocity_u_left);
    double partial_v_y = (velocity_v_top-velocity_v_bottom);

    double divergence = partial_u_x + partial_v_y;

    return divergence;
}

bool Fluid::is_solid(int x, int y)
{
    if(obstacle_grid[x][y] == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


// ---------------------------------------------------------------------------------


void Fluid::randomise_velocity_field(std::mt19937& random_generator)
{
    std::uniform_real_distribution<double> dist(0.0,10);

    for (int i = 0; i<velocity_grid_u.size();i++)
    {
        for (int j = 0; j<velocity_grid_u[i].size();j++)
        {
            double random_number =  dist(random_generator);
            velocity_grid_u[i][j] = random_number;
        }
    }

    for (int i = 0; i<velocity_grid_v.size();i++)
    {
        for(int j = 0; j<velocity_grid_v[j].size();j++)
        {
            double random_number = dist(random_generator);
            velocity_grid_v[i][j] = random_number;
        }
    }
}

void Fluid::vortex_shedding_obstacle()
{
    for(int i = 0; i<obstacle_grid.size();i++)
    {
        for(int j = 0; j<obstacle_grid[i].size();j++)
        {
            if((i == 0) || (j == 0) || (j == grid_size_y-1)){
                obstacle_grid[i][j] = 0;
            }
            else
            {
                obstacle_grid[i][j] = 1;
            }
        }
    }
}

void Fluid::box_setup()
{
    for(int i = 0; i<obstacle_grid.size();i++)
    {
        for(int j = 0; j<obstacle_grid[i].size();j++)
        {
            if((i == 0) || (i == grid_size_x-1) || (j == 0) || (j == grid_size_y-1))
            {
                obstacle_grid[i][j] = 0;
            }
            else
            {
                obstacle_grid[i][j] = 1;
            }
        }
    }
}
