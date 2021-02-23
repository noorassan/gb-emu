#pragma once

#include <cstdint>

#include "timer.h"

#define DIV 0xFF04
#define TIMA 0xFF05
#define TMA 0xFF06
#define TAC 0xFF07

class Bus;

class Timer {
public:
    Timer() = default;
    ~Timer() = default;

public:
    void connectBus(Bus *bus);
    void tick();

    uint8_t getTIMA();
    uint8_t getTMA();
    uint8_t getTAC();

    void setTIMA(uint8_t val);
    void incDIV();

private:
    uint16_t ticks;
    Bus *bus;
};