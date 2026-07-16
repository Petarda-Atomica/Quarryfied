#pragma once

#include <cstddef>
#include <utils/constrainedVec3.hpp>

#ifndef BASE_CHUNK_SIZE
#define BASE_CHUNK_SIZE 16
#endif

#ifndef BASE_REGION_SIZE
#define BASE_REGION_SIZE 32
#endif

constexpr std::size_t CHUNK_SIZE = BASE_CHUNK_SIZE;
constexpr std::size_t FLATTENED_CHUNK_SIZE = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

constexpr std::size_t REGION_SIZE = BASE_REGION_SIZE;
constexpr std::size_t FLATTENED_REGION_SIZE = REGION_SIZE * REGION_SIZE * REGION_SIZE;

using chunkCoord = ConstrainedVec3<CHUNK_SIZE>;
using regionCoor = ConstrainedVec3<REGION_SIZE>;
