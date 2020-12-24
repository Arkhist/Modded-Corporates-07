#include "vectorMath.hpp"

void copyVect(Vect3D* dest, Vect3D* src)
{
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}