#include "vectorMath.hpp"

#include <math.h>

void copyVect(Vect3D* dest, Vect3D* src)
{
    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;
}

void zeroVect(Vect3D* dest)
{
    dest->x = 0;
    dest->y = 0;
    dest->z = 0;
}

void scaleVect(Vect3D* dest, Vect3D* src, float scale)
{
    dest->x = src->x * scale;
    dest->y = src->y * scale;
    dest->z = src->z * scale;
}

float lengthVect(Vect3D* v)
{
    return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

void normalizeVect(Vect3D* dest, Vect3D* src)
{
    float factor = lengthVect(src);
    if (factor != 0)
        factor = 1/factor;
    scaleVect(dest, src, factor);
}

void getDirVectFromAngle(Vect3D* dest, float magnitude, float angle)
{
    angle += -M_PI_2;
    dest->y = 0;
    dest->x = cos(angle)*magnitude;
    dest->z = sin(angle)*magnitude;
}

void addVect(Vect3D* dest, Vect3D* a, Vect3D* b)
{
    dest->x = a->x + b->x;
    dest->y = a->y + b->y;
    dest->z = a->z + b->z;
}
