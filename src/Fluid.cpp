
#include <vector>
#include <cmath>
#include <iostream>

#include "Fluid.h"


Fluid::Fluid(size_t _grid_size_x, size_t _grid_size_y, double _cell_length ,double _time_step)
{
    grid_size_x = _grid_size_x;
    grid_size_y = _grid_size_y;
    cell_length = cell_length;
    time_step = _time_step;
    velocity_grid_u =  std::vector<std::vector<double>>(grid_size_y,std::vector<double>(grid_size_x+1,0));
    velocity_grid_v =  std::vector<std::vector<double>>(grid_size_y+1,std::vector<double>(grid_size_x,0));
    pressure_grid =  std::vector<std::vector<double>>(grid_size_y,std::vector<double>(grid_size_x,0));
}