#pragma once

#include <utils/enums.hpp>
#include "config.hpp"
#include <array>
#include <cstdint>

class RawChunk {
    std::array<uint16_t, FLATTENED_CHUNK_SIZE> blockArray;

    setBlockStatus setBlock(chunkCoord coords, uint16_t blockID);

    std::array<bool, CHUNK_SIZE * CHUNK_SIZE> faceMask(cardinalDirection dir) {
        std::array<bool, CHUNK_SIZE * CHUNK_SIZE> output = {};

        switch (dir) {
        case cardinalDirection::North:
            for (int y = 0; y < CHUNK_SIZE; y++)
                for (int x = 0; x < CHUNK_SIZE; x++)
                    output[x + CHUNK_SIZE * y] = static_cast<bool>(blockArray[chunkCoord(x, y, 0).flatten()]);
            break;

        case cardinalDirection::East:
            for (int y = 0; y < CHUNK_SIZE; y++)
                for (int z = 0; z < CHUNK_SIZE; z++)
                    output[z + CHUNK_SIZE * y] = static_cast<bool>(blockArray[chunkCoord(CHUNK_SIZE, y, z).flatten()]);
            break;

        case cardinalDirection::South:
            for (int y = 0; y < CHUNK_SIZE; y++)
                for (int x = 0; x < CHUNK_SIZE; x++)
                    output[x + CHUNK_SIZE * y] = static_cast<bool>(blockArray[chunkCoord(x, y, CHUNK_SIZE).flatten()]);
            break;

        case cardinalDirection::West:
            for (int y = 0; y < CHUNK_SIZE; y++)
                for (int z = 0; z < CHUNK_SIZE; z++)
                    output[z + CHUNK_SIZE * y] = static_cast<bool>(blockArray[chunkCoord(0, y, z).flatten()]);
            break;

        case cardinalDirection::Up:
            for (int z = 0; z < CHUNK_SIZE; z++)
                for (int x = 0; x < CHUNK_SIZE; x++)
                    output[x + CHUNK_SIZE * z] = static_cast<bool>(blockArray[chunkCoord(x, CHUNK_SIZE, z).flatten()]);
            break;

        case cardinalDirection::Down:
            for (int z = 0; z < CHUNK_SIZE; z++)
                for (int x = 0; x < CHUNK_SIZE; x++)
                    output[x + CHUNK_SIZE * z] = static_cast<bool>(blockArray[chunkCoord(x, 0, z).flatten()]);
            break;

        }

        return output;
    }
};
