#pragma once

#include "ankerl/unordered_dense.h"
#include "chunk/raw_chunk.hpp"
#include "chunk/palleted_chunk.hpp"
#include "chunk/sparse_chunk.hpp"
#include "chunk/config.hpp"
#include "utils/enums.hpp"
#include <array>

class Region {
private:
    // Maps used to store each chunk type
    ankerl::unordered_dense::map<regionCoord, RawChunk> rawRegion;
    ankerl::unordered_dense::map<regionCoord, PalletedChunk> palletedRegion;
    ankerl::unordered_dense::map<regionCoord, SparseChunk> sparseRegion;

    // Stores a number representing what chunk lies at each region coord
    std::array<chunkType, FLATTENED_REGION_SIZE> chunkLookup;
};
