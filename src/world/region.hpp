#pragma once

#include "ankerl/unordered_dense.h"
#include "BS_thread_pool.hpp"
#include "blocks.hpp"
#include "chunk/raw_chunk.hpp"
#include "chunk/palleted_chunk.hpp"
#include "chunk/sparse_chunk.hpp"
#include "chunk/config.hpp"
#include "glbinding/gl/types.h"
#include "glbinding/gl/functions.h" // IWYU pragma: keep
#include "spdlog/spdlog.h"
#include "utils/enums.hpp"
#include <array>
#include <cstdint>

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

    void renderAroundChunk(chunkCoord chunkCoords, uint_fast8_t renderDistance);
};
