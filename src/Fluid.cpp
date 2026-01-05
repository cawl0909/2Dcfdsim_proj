
#include <vector>
#include <cmath>
#include <iostream>
#include <random>

#include "Fluid.h"


Fluid::Fluid(size_t _grid_size_x, size_t _grid_size_y, double _cell_length ,double _time_step)
{
    grid_size_x = _grid_size_x;
    grid_size_y = _grid_size_y;
    cell_length = _cell_length;
    time_step = _time_step;
    velocity_grid_u =  std::vector<std::vector<double>>(grid_size_x+1,std::vector<double>(grid_size_y,0));
    velocity_grid_v =  std::vector<std::vector<double>>(grid_size_x,std::vector<double>(grid_size_y+1,0));
    pressure_grid =  std::vector<std::vector<double>>(grid_size_x,std::vector<double>(grid_size_y,0));
    obstacle_grid =  std::vector<std::vector<int>>(grid_size_x,std::vector<int>(grid_size_y,0)); //initialise empty boundary.

    //box 
    for(int i = 0;i<grid_size_x;i++)
    {
        obstacle_grid[i][0] = 1;
        obstacle_grid[i][grid_size_y-1] = 1;
    }
    for(int j = 0;j<grid_size_y;j++)
    {
        obstacle_grid[0][j] = 1;
        obstacle_grid[grid_size_x-1][j] = 1;
    }
}

// ---------------------------------------------------------------------------------

void Fluid::update_velocities()
{
    double constant = time_step/(fluid_density*cell_length);

    //updating u

    for(int i = 0; i<velocity_grid_u.size();i++)
    {
        for(int j = 0; j<velocity_grid_u[i].size();j++)
        {
            if((is_solid(i,j) == true) || (is_solid(i-1,j) == true))
            {
                velocity_grid_u[i][j] = 0;
                continue;
            }
            else
            {
                velocity_grid_u[i][j] = velocity_grid_u[i][j] - constant*(pressure_grid[i][j]-pressure_grid[i-1][j]);  
            }

        }
        
    }

    // updating v

    for(int i = 0; i<velocity_grid_v.size(); i++)
    {
        for(int j=0; j<velocity_grid_v[i].size();j++)
        {
            if((is_solid(i,j) == true) || (is_solid(i,j-1) == true))
            {
                velocity_grid_v[i][j] = 0;
                continue;
            }
            else
            {
                velocity_grid_v[i][j] = velocity_grid_v[i][j] - constant*(pressure_grid[i][j] - pressure_grid[i][j-1]);
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

    double partial_u_x = (velocity_u_right-velocity_u_left)/(cell_length); 
    double partial_v_y = (velocity_v_top-velocity_v_bottom)/(cell_length);

    double divergence = partial_u_x + partial_v_y;

    return divergence;
}

bool Fluid::is_solid(int x, int y)
{
    if(obstacle_grid[x][y] == 1)
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

