#pragma once

#include <cstdint>

enum class setBlockStatus : uint8_t {
    Ok = 0,
    NothingToDo = 1,
    NeedsPromotion = 2,
};

enum class chunkType : uint8_t {
    Empty    = 0,
    Raw      = 1,
    Palleted = 2,
    Sparse   = 3,
};

enum class cardinalDirection : uint8_t {
    North = 0,
    East  = 1,
    South = 2,
    West  = 3,
    Up    = 4,
    Down  = 5,
};
