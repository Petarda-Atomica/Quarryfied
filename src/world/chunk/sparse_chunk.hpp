#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <utils/enums.hpp>
#include "config.hpp"

struct SparseChunk {
    struct sparseEncoding {
        uint16_t blockID=0;
        std::vector<chunkCoord> coordinates{};
    };

    std::vector<sparseEncoding> blockArray{};

    setBlockStatus setBlock(ConstrainedVec3<CHUNK_SIZE> coords, uint16_t blockID);

    std::array<bool, CHUNK_SIZE * CHUNK_SIZE> faceMask(cardinalDirection dir) {
        std::array<bool, CHUNK_SIZE * CHUNK_SIZE> output = {};

        for (const auto& encoding : blockArray) {
            if (encoding.blockID == 0) {
                break;
            }
            for (const auto& coords : encoding.coordinates) {
                switch (dir) {
                case cardinalDirection::North:
                    if (coords.z != 0) continue;
                    output[coords.x + CHUNK_SIZE * coords.y] = true;
                    break;
                case cardinalDirection::East:
                    if (coords.x != CHUNK_SIZE) continue;
                    output[coords.z + CHUNK_SIZE * coords.y] = true;
                    break;
                case cardinalDirection::South:
                    if (coords.z != CHUNK_SIZE) continue;
                    output[coords.x + CHUNK_SIZE * coords.y] = true;
                    break;
                case cardinalDirection::West:
                    if (coords.x != 0) continue;
                    output[coords.z + CHUNK_SIZE * coords.y] = true;
                    break;
                case cardinalDirection::Up:
                    if (coords.y != CHUNK_SIZE) continue;
                    output[coords.x + CHUNK_SIZE * coords.z] = true;
                    break;
                case cardinalDirection::Down:
                    if (coords.y != 0) continue;
                    output[coords.x + CHUNK_SIZE * coords.z] = true;
                    break;
                }
            }
        }

        return output;
    }
};
