#include "networking.hpp"

#include <stdlib.h>
#include <stdio.h>

int startServer(void)
{
    int sockfd = -1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; 
    addr.sin_port = htons(27584);
  
    if (!bind(sockfd, (const struct sockaddr*)&addr, sizeof(addr))) {
        fcntl(sockfd, F_SETFL, O_NONBLOCK);
    }
    else {
        perror("bind:");
        sockfd = -1;
    }
    return sockfd;
}

int recvPacket(RecvContainer* ctn)
{
    int r = recvfrom(ctn->sockfd, ctn->rbuf, NETWORK_BUF_SIZE-1, 0, (struct sockaddr*)&ctn->addr, &ctn->len);
    ctn->addr.sin_port;
    ctn->readamt = r;
    ctn->readIndex = 0;
    ctn->readBitsInChar = 0;
    return r;
}

int32_t readNextInt4(RecvContainer* ctn)
{
    if (ctn->readBitsInChar > 0) {
        ctn->readBitsInChar = 0;
        ctn->readIndex++;
    }
    int32_t r = 0;
    for (int i = 0; i < 4; i++) {
        if (ctn->readIndex >= ctn->readamt)
            break;
        r += ctn->rbuf[ctn->readIndex] << (8*i);
        ctn->readIndex++;
    }
    return r;
}

uint8_t readNextByte(RecvContainer* ctn)
{
    if (ctn->readBitsInChar > 0) {
        ctn->readBitsInChar = 0;
        ctn->readIndex++;
    }
    uint8_t r = 0;
    if (ctn->readIndex >= ctn->readamt)
        return 0;
    r += ctn->rbuf[ctn->readIndex];
    ctn->readIndex++;
    
    return r;
}

int32_t readNextBitsInt4(int bits, RecvContainer* ctn)
{
    int32_t r = 0;
    for (int i = 0; i < bits; i++) {
        r |= (ctn->rbuf[ctn->readIndex] & (1 << ctn->readBitsInChar)) ? (1 << i) : 0;
        ctn->readBitsInChar++;
        if (ctn->readBitsInChar >= 8) {
            ctn->readBitsInChar = 0;
            ctn->readIndex++;
        } 
    }
    return r;
}

void resetSendCtn(RecvContainer* ctn)
{
    ctn->writtenBitsInChar = 0;
    ctn->writeIndex = 4;
    ctn->wbuf[0] = '7';
    ctn->wbuf[1] = 'D';
    ctn->wbuf[2] = 'F';
    ctn->wbuf[3] = 'P';
}

void sendPacket(RecvContainer* ctn)
{
    if(ctn->writtenBitsInChar > 0) {
        ctn->writeIndex++;
        ctn->writtenBitsInChar = 0;
    }
    sendto(ctn->sockfd, ctn->wbuf, ctn->writeIndex, 0, (struct sockaddr*)&ctn->addr, sizeof(struct sockaddr_in));
    ctn->writeIndex = 0;
}

void writeNextInt4(int32_t a, RecvContainer* ctn)
{
    if(ctn->writtenBitsInChar > 0) {
        ctn->writeIndex++;
        ctn->writtenBitsInChar = 0;
    }
    ctn->wbuf[ctn->writeIndex++] = (a & 0x000000FF) >>  0;
    ctn->wbuf[ctn->writeIndex++] = (a & 0x0000FF00) >>  8;
    ctn->wbuf[ctn->writeIndex++] = (a & 0x00FF0000) >> 16;
    ctn->wbuf[ctn->writeIndex++] = (a & 0xFF000000) >> 24;
}

void writeNextBuf(uint8_t* buf, int n, RecvContainer* ctn)
{
    if(ctn->writtenBitsInChar > 0) {
        ctn->writeIndex++;
        ctn->writtenBitsInChar = 0;
    }
    for(int i = 0; i < n; i++) {
        ctn->wbuf[ctn->writeIndex++] = buf[i];
    }
}

void writeNextByte(uint8_t byte, RecvContainer* ctn)
{
    if(ctn->writtenBitsInChar > 0) {
        ctn->writeIndex++;
        ctn->writtenBitsInChar = 0;
    }
    ctn->wbuf[ctn->writeIndex++] = byte;
}

void writeNextInt4Bits(int32_t a, int bits, RecvContainer* ctn)
{
    if (ctn->writtenBitsInChar == 0) {
        ctn->wbuf[ctn->writeIndex] = 0;
    }
    for (int i = 0; i < bits; i++) {
        ctn->wbuf[ctn->writeIndex] |= (a & (1 << i)) ? (1 << ctn->writtenBitsInChar) : 0;
        ctn->writtenBitsInChar++;
        if (ctn->writtenBitsInChar >= 8) {
            ctn->writtenBitsInChar = 0;
            ctn->writeIndex++;
            ctn->wbuf[ctn->writeIndex] = 0;
        }
    }
}

void writeNextInt8Bits(int64_t a, int bits, RecvContainer* ctn)
{
    if (ctn->writtenBitsInChar == 0) {
        ctn->wbuf[ctn->writeIndex] = 0;
    }
    for (int i = 0; i < bits; i++) {
        ctn->wbuf[ctn->writeIndex] |= (a & (1 << i)) ? (1 << ctn->writtenBitsInChar) : 0;
        ctn->writtenBitsInChar++;
        if (ctn->writtenBitsInChar >= 8) {
            ctn->writtenBitsInChar = 0;
            ctn->writeIndex++;
            ctn->wbuf[ctn->writeIndex] = 0;
        }
    }
}

void packPosition(Vect3D* vect, int bits, RecvContainer* ctn)
{
    uint32_t x = (uint32_t)((long)(vect->x * (float)(1 << ((uint8_t)bits & 31))));
    writeNextInt4Bits(x, bits + 12, ctn);
    x = (uint32_t)((long)(vect->y * (float)(1 << ((uint8_t)bits & 31))));
    writeNextInt4Bits(x, bits + 12, ctn);
    x = (uint32_t)((long)(vect->z * (float)(1 << ((uint8_t)bits & 31))));
    writeNextInt4Bits(x, bits + 12, ctn);
}

void packAngle(float f, int bits, RecvContainer* ctn)
{
    uint32_t r;
    uint32_t x;
    x = (uint32_t)(1 << (bits & 31)) >> 1;
    r = (uint)(long)(((float)(unsigned long)x / 6.283185) * (float)((uint32_t)f & 2147483647)) & x - 1;
    if (f < 0) {
        r = r | x;
    }
    writeNextInt4Bits(r, bits, ctn);
}
