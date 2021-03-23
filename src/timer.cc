#include "bus.h"
#include "interrupt.h"
#include "timer.h"

void Timer::connectBus(Bus *bus) {
    this->bus = bus;
}

void Timer::clock(uint8_t clocks) {
    // cycles will be at most 16, so it's not a problem to process all the cycles at once
    // because we'll never have two increments of TIMA per clock
    if (wait_cycles > 0 && (wait_cycles -= clocks) < 0) {
        write(TIMA, read(TMA));
        bus->requestInterrupt(INTERRUPT::TIMER);
    }

    internal_div += clocks;
    bool and_result = (read(TAC) & 0x03) &&
                      ((read(DIV) >> getDIVBitPos()) & 0x01);

    if (last_and_result && !and_result) {
        if (read(TIMA) == 0xFF) {
            overflowed = true;
            wait_cycles = 4;
        }

        write(TIMA, read(TIMA) + 1);
    }

    last_and_result = and_result;
}

uint8_t Timer::getDIVBitPos() {
    switch (read(TAC) & 0x03) {
        case 0:
            return 9;
        case 1:
            return 3;
        case 2:
            return 5;
        case 3:
            return 7;
    }

    return 0;
}

void Timer::write(uint16_t addr, uint8_t data) {
    bus->deviceWrite(addr, data);
}

uint8_t Timer::read(uint16_t addr) {
    return bus->deviceRead(addr);
}

bool Timer::regWrite(uint16_t addr, uint8_t data) {
    if (addr == DIV) {
        internal_div = 0;
        write(DIV, 0);
        return true;
    } else if (addr == TIMA) {
        wait_cycles = 0;
        write(TIMA, 0);
        return true;
    }

    return false;
}

bool Timer::regRead(uint16_t addr, uint8_t &val) {
    if (addr == DIV || addr == TIMA) {
        val = read(addr);
        return true;
    }

    return false;
}