#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>

const int PAYLOAD_SIZE = 1024;

struct Packet
{
    uint64_t seq;
    uint64_t timestamp;
    uint8_t type; // 1 = latency, 2 = bandwith
    char payload[PAYLOAD_SIZE];
};

#endif
