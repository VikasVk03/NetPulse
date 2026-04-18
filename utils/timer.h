#ifndef TIMER_H
#define TIMER_H

#include <chrono>

inline uint64_t now_us()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

#endif