#pragma once

#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

#include "human.hpp"

enum PlayerStatus {
    INVALID = 0,
    LOADING = 1,
    LOBBY = 2
};

enum PlayerMenu {
    NONE = 0,
    TEAM_SELECTION = 1,
    BUY_GENERAL = 2,
    BUY_WEAPONS = 3,
    BUY_VEHICLES = 4,
    BUY_EQUIPMENT = 5,
    BUY_STOCKS = 6
};

enum PlayerTeam {
    GOLDMEN = 0,
    MONSOTA = 1,
    OXS_INT = 2,
    OBSERVE = -1
};

struct PlayerAction {
    int actionType;
    int actionData;
};

struct Player {
    int id;
    int status;
    int isReady;

    char username[32];
    struct sockaddr_in addr;

    int cash;
    PlayerTeam team;

    int processedEvents;

    PlayerMenu currentMenu;

    int processedActions;
    int actions;
    PlayerAction actionData[64];

    Human* human;
};

struct Server;
struct RecvContainer;

Player* findPlayerFromRecv(Server* server, RecvContainer* ctn);

int registerPlayer(Server* server, RecvContainer* ctn);
int disconnectPlayer(Server* server, RecvContainer* ctn);