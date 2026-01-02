#ifndef VECTORS_H
#define VECTORS_H

#include<vector>
#include<iostream>

struct vector2d
{
    double x{};
    double y{};
};

std::ostream& operator<<(std::ostream& os, const vector2d& v);

double dot_product(const vector2d& vector1, const vector2d& vector2);

double norm(const vector2d& vector);

vector2d normalised(const vector2d& vector);

#endif