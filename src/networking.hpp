#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

#include "vectorMath.hpp"

#define NETWORK_BUF_SIZE 0x1000

enum RequestType {
    HEADER = 0,
    HEADER_RESPONSE = 1,
    REGISTER = 2,
    CLIENT_UPDATE = 4,
    CLIENT_RESPONSE = 5,
    DISCONNECT = 7
};

struct RecvContainer {
    int sockfd;

    unsigned char rbuf[NETWORK_BUF_SIZE];
    int readIndex;
    int readBitsInChar;
    ssize_t readamt;

    unsigned char wbuf[NETWORK_BUF_SIZE];
    int writtenBitsInChar;
    int writeIndex;

    struct sockaddr_in addr;
    socklen_t len;
};


int startServer(void);

int recvPacket(RecvContainer* ctn);

int32_t readNextInt4(RecvContainer* ctn);
int32_t readNextBitsInt4(int bits, RecvContainer* ctn);
uint8_t readNextByte(RecvContainer* ctn);

void sendPacket(RecvContainer* ctn);

void resetSendCtn(RecvContainer* ctn);
void writeNextInt4(int32_t a, RecvContainer* ctn);
void writeNextBuf(uint8_t* buf, int n, RecvContainer* ctn);
void writeNextByte(uint8_t byte, RecvContainer* ctn);
void writeNextInt4Bits(int32_t a, int bits, RecvContainer* ctn);
void writeNextInt8Bits(int64_t a, int bits, RecvContainer* ctn);

void packPosition(Vect3D* vect, int bits, RecvContainer* ctn);
void packAngle(float f, int bits, RecvContainer* ctn);
