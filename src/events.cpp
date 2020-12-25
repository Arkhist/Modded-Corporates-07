#include "events.hpp"

#include "server.hpp"

void packEvent(Event* event, RecvContainer* ctn)
{
    writeNextInt4Bits(event->id, 6, ctn);
    writeNextInt8Bits(event->tick, 28, ctn);
    switch(event->id) {
        case EventType::UPDATE_PLAYER:
        {
            writeNextInt4Bits(event->info.updatePlayer.playerId, 9, ctn);
            writeNextInt4Bits(event->info.updatePlayer.teamId, 4, ctn);
            writeNextInt4(event->info.updatePlayer.cashAmt, ctn);
            writeNextInt4Bits(event->info.updatePlayer.unknown, 8, ctn);
            for (int i = 0; i < 31; i++) {
                writeNextInt4Bits(event->info.updatePlayer.username[i] & 127, 7, ctn);
            }
        }
        break;
        case EventType::UPDATE_STOCKS:
        {
            writeNextBuf((uint8_t*)&event->info.updateStocks.goldmenStockVal, 4, ctn);
            writeNextBuf((uint8_t*)&event->info.updateStocks.monsotaStockVal, 4, ctn);
            writeNextBuf((uint8_t*)&event->info.updateStocks.OXSStockVal, 4, ctn);
        }
        break;
        default:
        break;
    }
}

void createEventUpdatePlayer(Server* server, Player* player)
{
    Event* ev = &server->events[server->eventNum];
    server->eventNum = (server->eventNum + 1) % EVENT_AMOUNT;
    ev->id = EventType::UPDATE_PLAYER;
    ev->tick = server->tickElapsed;
    ev->info.updatePlayer.playerId = player->id;
    ev->info.updatePlayer.teamId = player->team;
    ev->info.updatePlayer.cashAmt = player->cash;
    ev->info.updatePlayer.unknown = 2;
    int i = 0;
    for (; i < 31 && player->username[i] != '\0'; i++) {
        ev->info.updatePlayer.username[i] = player->username[i];
    }
    for (; i < 32; i++) {
        ev->info.updatePlayer.username[i] = 0;
    }
}

void createEventUpdateStocks(Server* server)
{
    Event* ev = &server->events[server->eventNum];
    server->eventNum = (server->eventNum + 1) % EVENT_AMOUNT;
    ev->id = EventType::UPDATE_STOCKS;
    ev->tick = server->tickElapsed;
    ev->info.updateStocks.goldmenStockVal = 200;
    ev->info.updateStocks.monsotaStockVal = 300;
    ev->info.updateStocks.OXSStockVal = 100;
    ev->info.updatePlayer.unknown = 0;
}
