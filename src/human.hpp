#pragma once

#include "vectorMath.hpp"

#define ACCEL_STRENGTH 10

struct Player;

struct Human {
    int isPlayed;

    int id;
    
    int health;
    int bleeding;

    float yaw;
    float pitch;
    Vect3D facingDirection;
    
    Vect3D position;
    Vect3D speed;
    Vect3D acceleration;

    Player* player;
};

struct Server;

Human* createHuman(Server* server, Vect3D position, Player* p);
