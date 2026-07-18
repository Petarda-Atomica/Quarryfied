#pragma once

#include "ankerl/unordered_dense.h"
#include "BS_thread_pool.hpp" // IWYU pragma: keep
#include "blocks.hpp"
#include "chunk/raw_chunk.hpp"
#include "chunk/palleted_chunk.hpp"
#include "chunk/sparse_chunk.hpp"
#include "chunk/config.hpp"
#include "glbinding/gl/types.h"
#include "glbinding/gl/functions.h" // IWYU pragma: keep
#include "glbinding/gl/bitfield.h"  // IWYU pragma: keep
#include "spdlog/spdlog.h" // IWYU pragma: keep
#include "utils/enums.hpp"
#include <vector>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath> // IWYU pragma: keep
#include <iterator> // IWYU pragma: keep
#include "graphics/structs.hpp"
#include "world/chunk/config.hpp"

class Region {
private:
    // Maps used to store each chunk type
    ankerl::unordered_dense::map<regionCoord, RawChunk> rawRegion;
    ankerl::unordered_dense::map<regionCoord, PalletedChunk> palletedRegion;
    ankerl::unordered_dense::map<regionCoord, SparseChunk> sparseRegion;

    // Stores a number representing what chunk lies at each region coord
    std::array<chunkType, FLATTENED_REGION_SIZE> chunkLookup;

    // Block manager for this region
    BlocksManager* Manager;

    // Shader Storage Buffer Objects
    GLuint MDIcmdsSSBO;
    GLuint seedsSSBO;

    // Workers
    void meshWorker(regionCoord regionCoords, std::vector<GPU::CubeFace> &faces, std::vector<GPU::DrawArraysIndirectCommand> &cmds);

public:
    // Constructor and destructor
    Region(BlocksManager* Manager);
    ~Region();

    // Prevent copying of this class
    Region(const Region&) = delete;
    Region& operator=(const Region&) = delete;

    // Set block
    setBlockStatus setBlock(localizedRegionCoord coords, uint16_t blockID);

    size_t renderAroundChunk(regionCoord chunkCoords, int_fast8_t renderDistance, int_fast8_t verticalRenderDistance);

    inline void draw(GLsizei drawingSize) {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, MDIcmdsSSBO);
        glMultiDrawArraysIndirect(
            GL_TRIANGLE_STRIP,
            (void*)0,
            drawingSize,
            0
        );
    }
};
