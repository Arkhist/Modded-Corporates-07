#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdexcept>
#include <bit>

#include <math.h>

#include "server.hpp"
#include "networking.hpp"

unsigned long systemtimer(void)
{
    timespec local_28 [2];
    
    clock_gettime(1,local_28);
    return (long)((double)(local_28[0].tv_nsec + local_28[0].tv_sec * 1000000000) * 1e-06) &
            0xffffffff;
}

void sendServerHeader(Server* server, RecvContainer* ctn)
{
    resetSendCtn(ctn);
    writeNextByte(RequestType::HEADER_RESPONSE, ctn);
    writeNextByte(server->version, ctn);
    writeNextBuf(ctn->rbuf+6, 4, ctn);
    writeNextInt4Bits(0, 4, ctn);
    writeNextInt4Bits(server->playerCount, 8, ctn);
    writeNextInt4Bits(server->maxPlayers, 8, ctn);
    writeNextBuf((uint8_t*)server->serverName, SERVER_NAME_LEN, ctn);
    sendPacket(ctn);
}

int maxSize = 0;

void handleClientUpdate(Server* server, RecvContainer* ctn)
{
    Player* p = findPlayerFromRecv(server, ctn);
    if (!p || p->status == PlayerStatus::INVALID)
        return;
    ctn->readIndex = 5;
    int curGame = readNextInt4(ctn);
    if (p->status == PlayerStatus::LOADING && curGame == server->curGame) {
        p->status = PlayerStatus::LOBBY;
    }
    readNextInt4(ctn); // idk2
    int tmp = readNextInt4(ctn);
    float val = *((float*)&tmp);
    p->controls.rightwards = val;
    readNextInt4(ctn); // idk1
    tmp = readNextInt4(ctn);
    val = *((float*)&tmp);
    p->controls.forwards = val;
    tmp = readNextInt4(ctn);
    val = *((float*)&tmp);
    p->controls.yaw = val;
    tmp = readNextInt4(ctn);
    val = *((float*)&tmp);
    p->controls.pitch = val;
    readNextInt4(ctn); // rv1
    readNextInt4(ctn); // rv2
    p->controls.inputStorage = readNextInt4(ctn);

    readNextBitsInt4(8, ctn); // Player Status
    p->processedEvents = readNextBitsInt4(16, ctn);
    int evtAmt = readNextBitsInt4(3, ctn);
    readNextBitsInt4(6, ctn);

    for (int i = 0; i < evtAmt; i++) {
        int x = readNextBitsInt4(4, ctn);
        if (x == 0) {
            int y = readNextBitsInt4(6, ctn);
            for (int j = 0; j < y; j++) {
                readNextByte(ctn);
            }
            readNextBitsInt4(4, ctn);
        }
    }

    evtAmt = readNextBitsInt4(4, ctn); // actions sent
    p->actions = readNextBitsInt4(8, ctn);

    for (int i = 0; i < evtAmt; i++) {
        p->actionData[p->actions].actionType = readNextBitsInt4(8, ctn);
        p->actionData[p->actions].actionData = readNextInt4(ctn);
        p->actions = (p->actions + 1) % 64;
    }

    // voice chat stuff here

}

void handlePacket(Server* server, RecvContainer* recvCtn)
{
    if (recvCtn->rbuf[0] != '7' || recvCtn->rbuf[1] != 'D' || recvCtn->rbuf[2] != 'F' || recvCtn->rbuf[3] != 'P')
        return;
    
    int requestType = recvCtn->rbuf[4];

    switch(requestType) {
    case RequestType::HEADER:
        sendServerHeader(server, recvCtn);
        break;
    case RequestType::REGISTER:
        registerPlayer(server, recvCtn);
        break;
    case RequestType::DISCONNECT:
        disconnectPlayer(server, recvCtn);
        break;
    case RequestType::CLIENT_UPDATE:
        handleClientUpdate(server, recvCtn);
        break;
    default:
        printf("Unknown packet type %d\n", requestType);
        break;
    }
}

void sendConnectionAnswer(Server* server, RecvContainer* ctn)
{
    resetSendCtn(ctn);
    writeNextByte(6, ctn);
    writeNextInt4(server->curGame, ctn);
    writeNextInt4Bits(server->gameMode, 4, ctn);
    writeNextInt4Bits(server->missionStyle, 4, ctn);
    writeNextByte('X', ctn); writeNextByte(0x90, ctn);
    writeNextByte(0, ctn); writeNextByte('\n', ctn);
    sendPacket(ctn);
}

void packItemsAndObjects(Server* server, RecvContainer* ctn)
{
    writeNextInt4Bits(0, 8, ctn);
    writeNextInt4Bits(0, 8, ctn);
}

void sendGameUpdateAnswer(Server* server, RecvContainer* ctn)
{
    Player* p = findPlayerFromRecv(server, ctn);

    resetSendCtn(ctn);
    writeNextByte(RequestType::CLIENT_RESPONSE, ctn);
    writeNextInt4(server->curGame, ctn);
    writeNextInt4(server->tickElapsed, ctn);
    writeNextInt4Bits(server->gameStatus, 4, ctn);
    writeNextInt4Bits(server->tickTimer, 16, ctn);
    writeNextInt4Bits(0, 16, ctn);
    writeNextInt4Bits(0, 6, ctn); // teams????
    writeNextInt4Bits(0, 6, ctn);
    writeNextInt4Bits(0, 6, ctn);
    writeNextInt4Bits(0, 6, ctn); // readPackets

    writeNextInt4Bits(p->id, 8, ctn);

    // idk
    writeNextInt4Bits(p->currentMenu, 8, ctn);
    writeNextInt4Bits(p->actions, 8, ctn);
    if (p->human) {
        writeNextInt4Bits(p->human->health, 8, ctn);
    }
    else {
        writeNextInt4Bits(0, 8, ctn);
    }
    writeNextInt4Bits(0, 6, ctn);
    writeNextInt4Bits(0, 8, ctn);


    // inventory
    writeNextInt4Bits(-1, 6, ctn); // HAND SLOT
    writeNextInt4Bits(-1, 6, ctn); // SLOT 1
    writeNextInt4Bits(-1, 6, ctn);
    writeNextInt4Bits(-1, 6, ctn);
    writeNextInt4Bits(-1, 6, ctn);
    writeNextInt4Bits(-1, 6, ctn);

    writeNextInt4Bits(0, 8, ctn);

    // idk
    for(int i = 0; i < MAX_ACTUAL_PLAYERS; i++) {
        writeNextInt4Bits((server->players[i].status != PlayerStatus::INVALID) ? 1 : 0, 1, ctn); // is player i played?
        writeNextInt4Bits(server->players[i].isReady, 1, ctn);
        if (server->players[i].human)
            writeNextInt4Bits(server->players[i].human->id, 8, ctn);
        else
            writeNextInt4Bits(-1, 8, ctn); // humanId (-1 = none)
    }

    for(int i = 0; i < MAX_ACTUAL_PLAYERS; i++) {
        writeNextInt4Bits(server->humans[i].isPlayed, 1, ctn);
        if (server->humans[i].isPlayed) {
            writeNextInt4Bits(-1, 8, ctn); // driven car
            writeNextInt4Bits(0, 2, ctn); // 0: driving hands, 1: right hand face forward
            packPosition(&server->humans[i].position, 12, ctn); // position
            writeNextInt4Bits(server->humans[i].health, 7, ctn);
            writeNextInt4Bits(server->humans[i].bleeding, 1, ctn);

            packAngle(0, 16, ctn); // angles
            packAngle(0, 16, ctn);
            packAngle(0, 16, ctn);
            packAngle(0, 16, ctn);
            packAngle(0, 16, ctn);
            packAngle(1.0, 16, ctn); // scrollAmount
            packAngle(server->humans[i].pitch, 16, ctn); // human pitch
            packAngle(server->humans[i].yaw, 16, ctn); // human yaw
        }
    }

    packItemsAndObjects(server, ctn);
    // compute voice
    for(int i = 0; i < 8; i++)
        writeNextInt4Bits(0, 1, ctn);
    
    int eventsToCatchUp = 0;
    if (server->eventNum < p->processedEvents) {
        eventsToCatchUp = (server->eventNum + 65536) - p->processedEvents;
    }
    else {
        eventsToCatchUp = server->eventNum - p->processedEvents;
    }
    if (eventsToCatchUp > 15) {
        eventsToCatchUp = 15;
    }

    writeNextInt4Bits(eventsToCatchUp, 4, ctn);
    writeNextInt4Bits(p->processedEvents, 16, ctn);

    int eventOffset = p->processedEvents;
    for (int i = 0; i < eventsToCatchUp; i++) {
        packEvent(&(server->events[eventOffset]), ctn);
        eventOffset = (eventOffset + 1) % EVENT_AMOUNT;
    }

    sendPacket(ctn);
}

void sendRoutine(Server* server, RecvContainer* ctn)
{
    for(int i = 0; i < 256; i++) {
        Player* p = server->players + i;
        if (p->status == PlayerStatus::INVALID) {
            continue;
        }
        ctn->addr = p->addr;
        ctn->len = sizeof(struct sockaddr_in);
        if (p->status == PlayerStatus::LOADING) {
            sendConnectionAnswer(server, ctn);
        }
        else if (p->status == PlayerStatus::LOBBY) {
            sendGameUpdateAnswer(server, ctn);
        }
    }
}

void endianCheck()
{
    if constexpr (std::endian::native != std::endian::little) {
        printf("WARNING: The program is probably more adapted to little endian machines. This machine is big endian.\n");
    }
}

void resetGame(Server* server)
{
    server->gameStatus = GameStatus::PREPARING;
    server->eventNum = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        Player* p = server->players + i;
        if (p->status == PlayerStatus::INVALID) {
            continue;
        }
        p->processedEvents = 0;
        
        p->processedActions = 0;
        p->actions = 0;

        p->isReady = 0;

        p->currentMenu = PlayerMenu::TEAM_SELECTION;
    }

    for (int i = 0; i < MAX_HUMANS; i++) {
        Human* h = server->humans + i;
        h->isPlayed = 0;
        h->player = nullptr;
    }
    createEventUpdateStocks(server);
}

void serverRecv(Server* server, RecvContainer* ctn)
{
    while(true) {
        recvPacket(ctn);
        if (ctn->readamt <= 0)
            return;
        handlePacket(server, ctn);
    }
}

void playerLogic(Server* server)
{

}

void playerActionLogic(Server* server, int playerId)
{
    Player* p = server->players + playerId;
    while (p->processedActions != p->actions) {
        PlayerAction* action = p->actionData + p->processedActions;
        switch (action->actionType) {
        case 1:
        {
            if (server->gameStatus != GameStatus::PREPARING)
                break;
            if (p->isReady)
                break;
            if (action->actionData == 1) {
                if (p->team == PlayerTeam::GOLDMEN)
                    break;
                p->team = PlayerTeam::GOLDMEN;
            }
            else if (action->actionData == 2) {
                if (p->team == PlayerTeam::MONSOTA)
                    break;
                p->team = PlayerTeam::MONSOTA;
            }
            else if (action->actionData == 3) {
                if (p->team == PlayerTeam::OXS_INT)
                    break;
                p->team = PlayerTeam::OXS_INT;
            }
            else if (action->actionData == 4) {
                if (p->team == PlayerTeam::OBSERVE)
                    break;
                p->team = PlayerTeam::OBSERVE;
            }
            else if (action->actionData == 5) { // ready
                if (p->isReady > 0)
                    break;
                p->isReady = 1;
            }
            else {
                break;
            }
            createEventUpdatePlayer(server, p);
            
        }
        break;
        default:
        break;
        }
        p->processedActions = (p->processedActions + 1) % 64;
    }

}

void gameLogic(Server* server)
{
    if (server->gameStatus == GameStatus::PLAYING) {
        server->tickTimer--;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (server->players[i].status == PlayerStatus::INVALID)
            continue;
        playerActionLogic(server, i);
    }

    if (server->gameStatus == GameStatus::PREPARING) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (server->players[i].status == PlayerStatus::INVALID)
                continue;
            if (server->players[i].isReady == 1 && !server->players[i].human) {
                createHuman(server, (Vect3D){512.0,64.0,512.0}, &server->players[i]);
                if (!server->players[i].human)
                    continue;
                server->players[i].isReady = 2;
                createEventUpdatePlayer(server, server->players + i);
                server->players[i].currentMenu = PlayerMenu::BUY_GENERAL;
            }
        }
    }
}

void humanSimulation(Server* server, unsigned long delta)
{
    float deltaf = (float)(delta)/1000.0;
    for (int i = 0; i < MAX_HUMANS; i++) {
        Human* h = server->humans + i;
        if (!h->isPlayed)
            continue;
        
        Player* controller = h->player;

        h->yaw = controller->controls.yaw;
        h->pitch = controller->controls.pitch;

        zeroVect(&h->acceleration);

        getDirVectFromAngle(&h->facingDirection, 1, h->yaw);
        copyVect(&h->acceleration, &h->facingDirection);
        scaleVect(&h->acceleration, &h->acceleration, controller->controls.forwards);

        Vect3D sideAccel;
        getDirVectFromAngle(&sideAccel, controller->controls.rightwards, h->yaw + M_PI_2);
        addVect(&h->acceleration, &h->acceleration, &sideAccel);

        normalizeVect(&h->acceleration, &h->acceleration);
        scaleVect(&h->acceleration, &h->acceleration, ACCEL_STRENGTH);

        scaleVect(&h->speed, &h->speed, 0.99); // damp speed

        Vect3D appliedVect;
        scaleVect(&appliedVect, &h->acceleration, deltaf);
        addVect(&h->speed, &h->speed, &appliedVect);
        
        scaleVect(&appliedVect, &h->speed, deltaf);
        addVect(&h->position, &h->position, &appliedVect);

        printf("Len: %f\n", lengthVect(&h->speed));
    }
}

/**
 * @brief PHYSICS! PHYSICS PHYSICS PHYSICS! PHY SICS!!!
 */
void physicsSimulation(Server* server, unsigned long delta)
{
    // particles
    // object
    humanSimulation(server, delta);
    // item sim
    // bond sim
    // item logic
    // item ttl
}

int main(int argc, const char **argv)
{
    endianCheck();
    Server* server = (Server*)calloc(1, sizeof(Server));
    server->maxPlayers = 32;
    server->playerCount = 0;
    strcpy(server->serverName, "Oh Seven Modded Corporates");
    server->version = 7;
    server->gameMode = 2;
    server->curGame = 2;
    server->missionStyle = MissionStyle::OTHER;
    server->tickTimer = 3600;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        server->players[i].status = PlayerStatus::INVALID;
        server->players[i].id = i;
    }
    for (int i = 0; i < MAX_HUMANS; i++) {
        server->humans[i].isPlayed = 0;
        server->humans[i].id = i;
    }
    resetGame(server);

    unsigned long time;
    int sockfd = startServer();
    if (sockfd == -1) {
        printf("Socket allocation failure\n");
        return 1;
    }
    printf("Server socket opened: %d\n", sockfd);
    server->sockfd = sockfd;

    // gen random
    // load accounts
    // setupgame

    // reset

    // timer
    RecvContainer ctn = {0};
    ctn.readamt = 0;
    ctn.sockfd = sockfd;

    printf("Entering main loop\n");

    unsigned long v5 = systemtimer();
    do {
        usleep(10000);
        unsigned long ms = systemtimer()-v5;
        
        serverRecv(server, &ctn);

        playerLogic(server);
        if (server->gameStatus != GameStatus::ENDING) {
            physicsSimulation(server, ms);
        }
        // physics logic

        gameLogic(server);

        sendRoutine(server, &ctn);

        server->tickElapsed++;
        v5 = systemtimer();
    } while(true);
}
