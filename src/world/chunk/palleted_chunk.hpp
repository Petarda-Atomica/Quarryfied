#pragma once

#include "config.hpp"
#include <utils/enums.hpp>
#include <array>
#include <cstdint>

struct PalletedChunk {
    std::array<uint16_t, 255> pallete{};
    std::array<uint8_t, FLATTENED_CHUNK_SIZE> blockArray{};

    inline uint8_t getPalletedBlockAt(chunkCoord coords) {
        return blockArray[coords.flatten()];
    }

    inline uint16_t getBlockAt(chunkCoord coords) {
        return pallete[this->getPalletedBlockAt(coords)];
    }

    setBlockStatus setBlock(chunkCoord coords, uint16_t blockID);

    std::array<bool, CHUNK_SIZE * CHUNK_SIZE> faceMask(cardinalDirection dir);
};
