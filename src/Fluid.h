#ifndef FLUID_H
#define FLUID_H

#include <vector>
#include <cmath>
#include <iostream>
#include <random>
#include <string>

#include "vectors.h"





class Fluid
{
public:
    double fluid_density;
    int i_numX;
    int i_numY;
    int numX;
    int numY;
    int numCells;
    double cell_size; // h
    double over_relaxation;
    std::vector<std::vector<double>> u_grid;
    std::vector<std::vector<double>> v_grid;
    std::vector<std::vector<double>> new_u_grid;
    std::vector<std::vector<double>> new_v_grid;
    std::vector<std::vector<double>> pressure;
    std::vector<std::vector<double>> solid;
    std::vector<std::vector<double>> mass;
    std::vector<std::vector<double>> new_mass;
    int num;

    Fluid(double _density, int _numX, int _numY, double _h, double _over_relaxation);

    void simulate(double dt, double grav, double num_iterations);

    double get_divergence(int x, int y); // helper function to get the divergence of the flow field

    void integrate(double dt, double gravity); // eulerian integration step to add gravity to all v veloicities in the grid

    void solveIncompressability(int numIterations, double dt); // solves pressure and veloicties by setting divergence to 0 of all real cells because of incompressability div.(u,v)  = 0

    void border_velocity_extrapolate(); // need to use ghost edge cells to deal with the simulated region margins, so appropriate veloicties are extrapolated from neighbours

    double grid_interpolation(double x, double y, std::string field); //does a bivariate interpolation on a chosen field for advection fields

    double get_avg_u(int x, int y);

    double get_avg_v(int x, int y);
    
    void advect_velocity(double dt);

    void reset_pressure();
    // Test functions


    void randomise_velocities(std::mt19937& generator);
};



#endif