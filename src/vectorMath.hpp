#pragma once

struct Vect3D {
    float x;
    float y;
    float z;
};

void copyVect(Vect3D* dest, Vect3D* src);
void zeroVect(Vect3D* dest);

void scaleVect(Vect3D* dest, Vect3D* src, float scale);
float lengthVect(Vect3D* v);
void normalizeVect(Vect3D* dest, Vect3D* src);

void getDirVectFromAngle(Vect3D* dest, float magnitude, float angle);

void addVect(Vect3D* dest, Vect3D* a, Vect3D* b);

