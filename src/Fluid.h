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

    double get_divergence(int x, int y);

    void integrate(double dt, double gravity);

    void solveIncompressability(int numIterations, double dt);
};



#endif