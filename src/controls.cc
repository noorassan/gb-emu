#include "bus.h"
#include "controls.h"

Controls::Controls(GameboyDriver *driver) {
    this->driver = driver;
    pressed = 0;
}

#include <iostream>
bool Controls::regRead(uint16_t addr, uint8_t &val) {
    if (addr == P1) {
        pollControls();

        uint8_t p1 = read(P1);
        val = 0;

        if (~p1 & 0x10) {
            // Direction buttons selected
            val |= ((pressed >> 4) & 0x0F);
        }

        if (~p1 & 0x20) {
            // Action buttons selected
            val |= ((pressed >> 0) & 0x0F);
        }

        val = ~val & 0x0F;
        val |= p1 & 0xF0;

        return true;
    }

    return false;
}

bool Controls::regWrite(uint16_t addr, uint8_t data) {
    if (addr == P1) {
        write(addr, data);

        if (~data & 0x10) {
            // Direction buttons selected
            pressed &= 0xF0;
        }

        if (~data & 0x20) {
            // Action buttons selected
            pressed &= 0x0F;
        }

        return true;
    }

    return false;
}

void Controls::pollControls() {
    uint8_t inputs = driver->pollControls();

    uint8_t new_pressed = pressed | inputs;

    // Joypad interrupt triggered on input
    if (new_pressed != pressed) {
        bus->requestInterrupt(INTERRUPT::JOYPAD);
    }

    pressed = new_pressed;
}

uint8_t Controls::read(uint16_t addr) {
    return bus->deviceRead(addr);
}

void Controls::write(uint16_t addr, uint8_t data) {
    bus->deviceWrite(addr, data);
}

void Controls::connectBus(Bus *bus) {
    this->bus = bus;
}