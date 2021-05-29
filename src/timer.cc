#include "bus.h"
#include "interrupt.h"
#include "timer.h"

void Timer::connectBus(Bus *bus) {
    this->bus = bus;
}

void Timer::clock(uint8_t clocks) {
    while (clocks > 0) {
        uint8_t curr_clocks = clocks > 16 ? 16 : clocks;
        clocks -= curr_clocks;

        if (wait_cycles > 0 && wait_cycles < curr_clocks) {
            write(TIMA, read(TMA));
            bus->requestInterrupt(INTERRUPT::TIMER);
            wait_cycles = 0;
        } else if (wait_cycles > 0) {
            wait_cycles -= curr_clocks;
        }

        internal_div += curr_clocks;
        write(DIV, internal_div >> 8);
    
        bool and_result = (read(TAC) & 0x04) &&
                          ((read(DIV) >> getDIVBitPos()) & 0x01);

        if (last_and_result && !and_result) {
            if (read(TIMA) == 0xFF) {
                wait_cycles = 4;
            }

            write(TIMA, read(TIMA) + 1);
        }

        last_and_result = and_result;
    }
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
        write(TIMA, data);
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