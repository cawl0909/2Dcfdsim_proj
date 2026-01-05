#ifndef FLUID_H
#define FLUID_H

#include <vector>
#include <cmath>
#include <iostream>

#include "vectors.h"

class Fluid
{
public:
    double fluid_density;
    int numX;
    int numY;
    int numCells;
    double cell_size; // h
    double over_relaxation;
    std::vector<double> u_grid;
    std::vector<double> v_grid;
    std::vector<double> new_u_grid;
    std::vector<double> new_v_grid;
    std::vector<double> pressure;
    std::vector<double> solid;
    std::vector<double> mass;
    std::vector<double> new_mass;
    int num;

    Fluid(double _density, int _numX, int _numY, double _h, double _over_relaxation);

    void integrate(double dt, double gravity);

    void solveIncompressability(int numIterations, double dt);
};



#endif