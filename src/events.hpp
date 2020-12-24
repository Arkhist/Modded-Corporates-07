#pragma once

#include "networking.hpp"

enum EventType {
    UPDATE_PLAYER = 8,
    UPDATE_STOCKS = 13
};

struct Event_UpdatePlayer {
    int32_t playerId;
    int32_t teamId;
    int32_t cashAmt;
    int32_t unknown;
    char username[32];
};

struct Event_UpdateStocks {
    float goldmenStockVal;
    float monsotaStockVal;
    float OXSStockVal;
};

union EventData {
    Event_UpdatePlayer updatePlayer;
    Event_UpdateStocks updateStocks;
};

struct Event {
    EventType id;
    uint64_t tick;
    
    EventData info;
};

void packEvent(Event* event, RecvContainer* ctn);

struct Server;
struct Player;

void createEventUpdatePlayer(Server* server, Player* player);
void createEventUpdateStocks(Server* server);
