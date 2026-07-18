#pragma once

#include <utils/enums.hpp>
#include <graphics/structs.hpp>
#include "config.hpp"
#include <array>
#include <cstdint>
#include <blocks.hpp>
#include <ankerl/unordered_dense.h>

class RawChunk {
    std::array<uint16_t, FLATTENED_CHUNK_SIZE> blockArray;

public:
    setBlockStatus setBlock(chunkCoord coords, uint16_t blockID);

    std::array<bool, CHUNK_SIZE * CHUNK_SIZE> faceMask(cardinalDirection dir);

    void mesh(std::vector<GPU::CubeFace>& faces, std::vector<GPU::DrawArraysIndirectCommand>& cmds, regionCoord regionCoord, std::array<std::array<bool, CHUNK_SIZE * CHUNK_SIZE>, 6> neighboursFaceMask, BlocksManager* blocksManager);
};
