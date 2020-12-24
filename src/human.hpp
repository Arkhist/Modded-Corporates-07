#pragma once

#include "vectorMath.hpp"

struct Player;

struct Human {
    int isPlayed;

    int id;
    
    int health;
    int bleeding;
    
    Vect3D position;

    Player* player;
};

struct Server;

Human* createHuman(Server* server, Vect3D position, Player* p);
