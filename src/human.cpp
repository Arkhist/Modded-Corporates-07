#include "human.hpp"

#include "server.hpp"
#include "player.hpp"

Human* createHuman(Server* server, Vect3D position, Player* p)
{
    Human* humanSlot = nullptr;
    int i = 0;
    for (; i < MAX_ACTUAL_PLAYERS; i++) {
        humanSlot = server->humans + i;
        if (humanSlot->isPlayed)
            continue;
        break;
    }
    if (i == MAX_ACTUAL_PLAYERS)
        return nullptr;
    humanSlot->player = p;
    p->human = humanSlot;

    humanSlot->health = 100;
    humanSlot->bleeding = 0;
    humanSlot->isPlayed = 1;

    copyVect(&humanSlot->position, &position);
    zeroVect(&humanSlot->speed);
    zeroVect(&humanSlot->acceleration);
    zeroVect(&humanSlot->facingDirection);

    return humanSlot;
}
