#pragma once

#include <cstdint>

typedef union {
    struct {
        unsigned a : 1;
        unsigned b : 1;
        unsigned select : 1;
        unsigned start : 1;
        unsigned right : 1;
        unsigned left: 1;
        unsigned up: 1;
        unsigned down: 1;
    };
    uint8_t data;
} ControllerState;