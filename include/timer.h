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
    Timer();
    ~Timer() = default;

public:
    void clock(uint8_t cycles);
    void reset();

    // Returns true if a r/w to addr is handled by the timer
    bool regWrite(uint16_t addr, uint8_t data);
    bool regRead(uint16_t addr, uint8_t &val);

    void connectBus(Bus *bus);

private:
    uint8_t getDIVBitPos();

    // timer registers
    uint16_t internal_div;
    uint8_t tima;
    uint8_t tma;
    uint8_t tac;

    uint8_t wait_cycles;
    bool last_and_result;
    Bus *bus;
};