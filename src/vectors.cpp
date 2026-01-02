#include "vectors.h"

#include <cmath>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const vector2d& v)
{
    os<<"("<<v.x<<","<<v.y<<")";
    return os;
}

double dot_product(const vector2d& vector1, const vector2d& vector2)
{
    double dot_product_sum = (vector1.x*vector2.x) + (vector1.y*vector2.y);
    return dot_product_sum;
}

double norm(const vector2d& vector)
{
    double length = std::pow((std::pow(vector.x,2)+std::pow(vector.y,2)),(1/2));
    return length;
}

vector2d normalised(const vector2d& vector)
{
    vector2d temp_vector = vector;
    double inverse_len = 1.0/norm(vector);
    temp_vector.x = temp_vector.x*inverse_len;
    temp_vector.y = temp_vector.y*inverse_len;
    return temp_vector;
}
