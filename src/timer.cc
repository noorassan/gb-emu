#include "bus.h"
#include "interrupt.h"
#include "timer.h"

void Timer::connectBus(Bus *bus) {
    this->bus = bus;
}

void Timer::tick() {
    uint8_t tac = getTAC();
    uint8_t tima = getTIMA();

    bool inc = false;
    switch(tac & 0x07) {
        case 4: // 4.096 kHz
            inc = ticks % 1024 == 0;
        case 5: // 262.144 KHz
            inc = ticks % 16 == 0;
        case 6: // 65.536 KHz
            inc = ticks % 64 == 0;
        case 7: // 16.384 KHz
            inc = ticks % 256 == 0;
    }

    if (inc) {
        if (tima == 0xFF) {
            bus->requestInterrupt(TIMER);
            setTIMA(getTMA());            
        } else {
            setTIMA(tima + 1);
        }
    }

    if (ticks % 256 == 0) {
        incDIV();
    }
}

uint8_t Timer::getTAC() {
    return bus->read(TAC);
}

uint8_t Timer::getTIMA() {
    return bus->read(TIMA);
}

uint8_t Timer::getTMA() {
    return bus->read(TMA);
}

void Timer::setTIMA(uint8_t val) {
    bus->write(TIMA, val);
}

void Timer::incDIV() {
    bus->write(DIV, bus->read(DIV) + 1);
}