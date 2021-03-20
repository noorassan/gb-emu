#pragma once

#include "controls.h"
#include "color.h"

typedef void (*DrawFn)(COLOR color, uint8_t x, uint8_t y);
typedef CONTROL (*PollControlsFn)();