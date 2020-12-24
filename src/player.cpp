#include "player.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "server.hpp"
#include "networking.hpp"

Player* findPlayerFromRecv(Server* server, RecvContainer* ctn)
{
    Player* playerSlot;
    for(int pId = 0; pId < 256; pId++) {
        playerSlot = server->players + pId;

        if (playerSlot->addr.sin_addr.s_addr == ctn->addr.sin_addr.s_addr &&
            playerSlot->addr.sin_port == ctn->addr.sin_port) {

            return playerSlot;
        }
    }
    return nullptr;
}

int registerPlayer(Server* server, RecvContainer* ctn)
{
    if (ctn->rbuf[5] != server->version)
        return 1;

    if (server->playerCount >= server->maxPlayers)
        return 2;

    int firstEmptySlot = -1;
    Player* playerSlot;
    for(int pId = 0; pId < 32; pId++) {
        playerSlot = server->players + pId;

        if (playerSlot->status == PlayerStatus::INVALID && firstEmptySlot == -1)
            firstEmptySlot = pId;
        else {
            if (playerSlot->addr.sin_addr.s_addr == ctn->addr.sin_addr.s_addr &&
                playerSlot->addr.sin_port == ctn->addr.sin_port)
                return 3;
        }
    }
    playerSlot = server->players + firstEmptySlot;

    memset(playerSlot->username, 0, 32);

    // Username parsing
    int i = 0;
    for (i = 0; i < 31; i++) {
        char c = ctn->rbuf[i+6];
        if (!(c == 32 || (c >= 48 && c <= 57) || (c >= 65 && c <= 122)))
            break;
        playerSlot->username[i] = c;
    }

    printf("Player %s connected\n", playerSlot->username);

    playerSlot->addr = ctn->addr;

    playerSlot->status = PlayerStatus::LOADING;
    playerSlot->id = firstEmptySlot;
    playerSlot->cash = 150;
    playerSlot->processedEvents = 0;

    playerSlot->isReady = 0;
    playerSlot->human = NULL;
    
    playerSlot->team = PlayerTeam::OBSERVE;

    if (server->gameStatus == GameStatus::PREPARING) {
        playerSlot->currentMenu = PlayerMenu::TEAM_SELECTION;
    }
    else {
        playerSlot->currentMenu = PlayerMenu::NONE;
    }

    playerSlot->processedActions = 0;
    playerSlot->actions = 0;

    server->playerCount++;

    createEventUpdatePlayer(server, playerSlot);

    return 0;
}

int disconnectPlayer(Server* server, RecvContainer* ctn)
{
    Player* p;
    if (p = findPlayerFromRecv(server, ctn)) {
        p->status = PlayerStatus::INVALID;
        printf("Player %s disconnected\n", p->username);

        server->playerCount--;
        return 0;
    }
    return 1;
}
