#include "bus.h"
#include "controls.h"

Controls::Controls(GameboyDriver *driver) {
    this->driver = driver;
    pressed.data = 0;
}

bool Controls::regRead(uint16_t addr, uint8_t &val) {
    if (addr == P1) {
        updateControls();

        val = 0;

        if (~p1 & 0x10) {
            // Direction buttons selected
            val |= ((pressed.data >> 4) & 0x0F);
        }

        if (~p1 & 0x20) {
            // Action buttons selected
            val |= ((pressed.data >> 0) & 0x0F);
        }

        val = ~val & 0x0F;
        val |= p1 & 0xF0;

        return true;
    }

    return false;
}

bool Controls::regWrite(uint16_t addr, uint8_t data) {
    if (addr == P1) {
        p1 = data;
        return true;
    }

    return false;
}

void Controls::updateControls() {
    ControllerState new_pressed = driver->pollControls();

    // Joypad interrupt triggered on input
    if (new_pressed.data != pressed.data) {
        bus->requestInterrupt(INTERRUPT::JOYPAD);
    }

    pressed = new_pressed;
}

void Controls::connectBus(Bus *bus) {
    this->bus = bus;
}