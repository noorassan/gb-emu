#pragma once

#include <cstdint>

#include "gb_driver.h"
#include "interrupt.h"

#define P1 0xFF00

class Bus;


class Controls {
public:
    Controls(GameboyDriver *driver);
    ~Controls() = default;

public:
    // Returns true if a r/w to addr is handled by Controls
    bool regRead(uint16_t addr, uint8_t &val);
    bool regWrite(uint16_t addr, uint8_t data);

    void connectBus(Bus *bus);

    // Updates P1 register based on controller inputs
    void updateControls();

private:
    Bus *bus;
    GameboyDriver *driver;

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

    ControllerState pressed;
};