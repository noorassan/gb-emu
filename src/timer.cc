#include "bus.h"
#include "interrupt.h"
#include "timer.h"

Timer::Timer() {
    reset();
}

void Timer::clock(uint8_t clocks) {
    while (clocks > 0) {
        uint8_t curr_clocks = (clocks > 4) ? 4 : clocks;
        clocks -= curr_clocks;

        if (wait_cycles > 0 && wait_cycles <= curr_clocks) {
            tima = tma;
            bus->requestInterrupt(INTERRUPT::TIMER);
            wait_cycles = 0;
        } else if (wait_cycles > 0) {
            wait_cycles -= curr_clocks;
        }

        internal_div += curr_clocks;
    
        bool and_result = (tac & 0x04) &&
                          ((internal_div >> getDIVBitPos()) & 0x01);

        if (last_and_result && !and_result) {
            if (tima == 0xFF) {
                wait_cycles = 4;
            }

            tima++;
        }

        last_and_result = and_result;
    }
}

void Timer::reset() {
    internal_div = 0xABCC;
    tima = 0x00;
    tma = 0x00;
    tac = 0x00;

    wait_cycles = 0;
    last_and_result = false;
}

bool Timer::regWrite(uint16_t addr, uint8_t data) {
    switch(addr) {
        case DIV:
            internal_div = 0;
            break;
        case TIMA:
            wait_cycles = 0;
            tima = data;
            break;
        case TMA: tma = data; break;
        case TAC: tac = data; break;
        default:  return false;
    }

    return true;
}

bool Timer::regRead(uint16_t addr, uint8_t &val) {
    switch(addr) {
        case DIV:
            val = internal_div >> 8;
            break;
        case TIMA: val = tima; break;
        case TMA:  val = tma;  break;
        case TAC:  val = tac;  break;
        default:   return false;
    }

    return true;
}

void Timer::connectBus(Bus *bus) {
    this->bus = bus;
}

uint8_t Timer::getDIVBitPos() {
    switch (tac & 0x03) {
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