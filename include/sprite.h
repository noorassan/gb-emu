#pragma once

#include <cstdint>

class Sprite {
public:
    Sprite() = default;
    ~Sprite() = default;

public:
    uint8_t pos_y;
    uint8_t pos_x;
    uint8_t tile_num;
    uint8_t flags;

    inline bool operator<(const Sprite &other) const { return pos_x < other.pos_x; }
    inline bool operator>(const Sprite &other) const { return other.operator<(*this); }
};
