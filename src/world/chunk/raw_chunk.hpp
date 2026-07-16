#pragma once

#include <utils/enums.hpp>
#include "config.hpp"
#include <array>
#include <cstdint>

class RawChunk {
    std::array<uint16_t, FLATTENED_CHUNK_SIZE> blockArray;

    setBlockStatus setBlock(chunkCoord coords, uint16_t blockID);

    std::array<bool, CHUNK_SIZE * CHUNK_SIZE> faceMask(cardinalDirection dir);
};
