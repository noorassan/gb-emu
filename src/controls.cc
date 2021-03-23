#include "bus.h"
#include "controls.h"

Controls::Controls(GameboyDriver *driver) {
    this->driver = driver;
}

bool Controls::regRead(uint16_t addr, uint8_t &val) {
    if (addr == P1) {
        pollControls();
        val = read(P1);
        return true;
    }

    return false;
}

bool Controls::regWrite(uint16_t addr, uint8_t data) {
    if (addr == P1) {
        write(addr, data);
        return true;
    }

    return false;
}

void Controls::pollControls() {
    uint8_t p1 = read(P1);
    uint8_t inputs = driver->pollControls(p1);
    write(P1, (~inputs & 0x0F) | (p1 & 0xF0));
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