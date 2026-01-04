#ifndef FLUID_H
#define FLUID_H

#include <vector>
#include <cmath>
#include <iostream>

#include "vectors.h"

class Fluid
{
public:
    size_t grid_size_x;
    size_t grid_size_y;
    double cell_length;

    double time_step;

    std::vector<std::vector<double>> velocity_grid_u;
    std::vector<std::vector<double>> velocity_grid_v;

    std::vector<std::vector<double>> pressure_grid;

    Fluid(size_t _grid_size_x, size_t _grid_size_y, double _cell_length, double _time_step);

    void advect();
    

};



#endif