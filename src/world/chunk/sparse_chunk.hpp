#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <utils/enums.hpp>
#include "blocks.hpp"
#include "config.hpp"
#include "graphics/structs.hpp"

struct SparseChunk {
    struct sparseEncoding {
        uint16_t blockID=0;
        std::vector<chunkCoord> coordinates{};
    };
    std::vector<sparseEncoding> blockArray{};

public:
    setBlockStatus setBlock(ConstrainedVec3<CHUNK_SIZE> coords, uint16_t blockID);

    std::array<bool, CHUNK_SIZE * CHUNK_SIZE> faceMask(cardinalDirection dir);

    void mesh(std::vector<GPU::CubeFace>& faces, std::vector<GPU::DrawArraysIndirectCommand>& cmds, regionCoord regionCoord, std::array<std::array<bool, CHUNK_SIZE * CHUNK_SIZE>, 6> neighboursFaceMask, BlocksManager* blocksManager);
};
