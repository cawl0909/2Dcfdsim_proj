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
    double fluid_density = 1;

    double time_step;

    std::vector<std::vector<double>> velocity_grid_u;
    std::vector<std::vector<double>> velocity_grid_v;

    std::vector<std::vector<double>> pressure_grid;

    std::vector<std::vector<int>> obstacle_grid;

    Fluid(size_t _grid_size_x, size_t _grid_size_y, double _cell_length, double _time_step);


    // Main split solver loop
    void advect();
    void update_velocities();
    void pressure_solve();

    // Calculation helper functions
    double calculate_velocity_divergence(int x, int y);
    bool is_solid(int x, int y);

    // Fluid test functions

    void randomise_velocity_field(std::mt19937& random_generator);
};



#endif