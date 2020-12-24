#pragma once

#include <stdint.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

#include "player.hpp"
#include "events.hpp"

#define MAX_PLAYERS 256
#define MAX_HUMANS 512
#define SERVER_NAME_LEN 32

#define MAX_ACTUAL_PLAYERS 32

#define EVENT_AMOUNT 65536

enum GameStatus {
    PREPARING = 1,
    PLAYING = 2,
    ENDING = 3
};

enum MissionStyle {
    OTHER = 0,
    ACQUISITION = 1,
    RACE = 2
};

struct Server {
    int sockfd;
    uint8_t version;
    uint64_t tickElapsed;
    

    int maxPlayers;
    int playerCount;

    char serverName[SERVER_NAME_LEN];

    int gameMode;
    int curGame;
    GameStatus gameStatus;
    MissionStyle missionStyle;
    unsigned int tickTimer;

    int eventNum;
    Event events[EVENT_AMOUNT];

    Player players[MAX_PLAYERS];
    Human humans[MAX_HUMANS];
};
