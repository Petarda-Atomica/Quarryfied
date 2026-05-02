#pragma once

#include "spdlog/spdlog.h"
#include <array>
#include <bit>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <ankerl/unordered_dense.h>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

template<size_t CHUNK_SIZE, size_t REGION_SIZE>
class ChunkManager {
public:
    #pragma pack(push, 1)
    template<size_t SIZE>
    struct ConstrainedVec3 {
        // --- Compile time shenanigans ---
        // Find how many bits we need
        static constexpr size_t BITS = std::bit_width(SIZE - 1);
        static constexpr size_t TOTAL_BITS = BITS * 3;

        // Make sure we can fit everything in an uint64_t
        static_assert(TOTAL_BITS <= 64,
            "Chunk coordinates exceed 64 bit storage limit! Reduce SIZE."
        );

        // Determine best vessel to hold the data
        using StorageType =
            std::conditional_t<(TOTAL_BITS <= 8), uint8_t,
            std::conditional_t<(TOTAL_BITS <= 16), uint16_t,
            std::conditional_t<(TOTAL_BITS <= 32), uint32_t,
            uint64_t
        >>>;

        // --- Actual data ---
        StorageType x : BITS;
        StorageType y : BITS;
        StorageType z : BITS;

        // --- Functions ---
        // Stuff to figure out how to flatten this
        static constexpr size_t FLATTENED_BITS = std::bit_width(SIZE*SIZE*SIZE);
        static_assert(FLATTENED_BITS <= 64,
            "Flattened chunk coordinates exceed 64 bit storage limit! Reduce SIZE."
        );
        using FlattenedType =
            std::conditional_t<(FLATTENED_BITS <= 8), uint8_t,
            std::conditional_t<(FLATTENED_BITS <= 16), uint16_t,
            std::conditional_t<(FLATTENED_BITS <= 32), uint32_t,
            uint64_t
        >>>;
        constexpr FlattenedType flatten() const {
            return  (static_cast<FlattenedType>(z) * SIZE * SIZE) +
                    (static_cast<FlattenedType>(y) * SIZE) +
                    static_cast<FlattenedType>(x);
        }

        // Generic constructor
        ConstrainedVec3() = default;
        // Specific constructor
        ConstrainedVec3(const int x, const int y, const int z) :
            x(static_cast<StorageType>(x)),
            y(static_cast<StorageType>(y)),
            z(static_cast<StorageType>(z)) {}
    };
    #pragma pack(pop)

    // Stuff for hashing coordinates
    template<size_t SIZE>
    struct ConstrainedVec3Hasher {
        using is_avalanching = void;

        size_t operator()(const ChunkManager<CHUNK_SIZE, REGION_SIZE>::ConstrainedVec3<SIZE>& v) const noexcept {
            // We perform the flattening logic here instead of inside the struct
            return static_cast<size_t>(v.z) * SIZE * SIZE +
                static_cast<size_t>(v.y) * SIZE +
                static_cast<size_t>(v.x);
        }
    };

    // Stuff for hashing coordinates
    template<size_t SIZE>
    struct ConstrainedVec3Equal {
        bool operator()(const ChunkManager<CHUNK_SIZE, REGION_SIZE>::ConstrainedVec3<SIZE>& lhs,
                        const ChunkManager<CHUNK_SIZE, REGION_SIZE>::ConstrainedVec3<SIZE>& rhs) const {
            return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
        }
    };

    std::pair<uint16_t, uint8_t> getChunkBlock(ConstrainedVec3<REGION_SIZE> regionCoord, ConstrainedVec3<CHUNK_SIZE> chunkCoord, uint8_t knownNeighbour = 0) {
        if (knownNeighbour) {
            switch (knownNeighbour) {
            case 1:
                return {rawLocalRegion[regionCoord.flatten()].blockArray[chunkCoord.flatten()], 1};
                break;
            case 2: {
                auto& chunk = palletedLocalRegion[regionCoord.flatten()];
                auto blockID = chunk.blockArray[chunkCoord.flatten()];
                return {chunk.pallete[blockID], 2};
            }
            case 3: {
                auto& chunk = sparseLocalRegion[regionCoord.flatten()];
                for (auto& refrence : chunk) {
                    if (refrence.coordinates.contains(chunkCoord))
                        return {refrence.coordinates[chunkCoord], 3};
                }
            }
            }
        } else {
            if (rawLocalRegion.contains(regionCoord.flatten()))
                return rawLocalRegion[regionCoord.flatten()].blockArray[chunkCoord.flatten()];
            else if (palletedLocalRegion.contains(regionCoord.flatten()))
                return getChunkBlock(regionCoord, chunkCoord, 2);
            else if (sparseLocalRegion.contains(regionCoord.flatten()))
                return getChunkBlock(regionCoord, chunkCoord, 3);
        }
    }

    std::pair<uint16_t, uint8_t> getBlock(ConstrainedVec3<CHUNK_SIZE * REGION_SIZE> coords, uint8_t knownNeighbour = 0) {
        ConstrainedVec3<REGION_SIZE> regionCoord(coords.x / REGION_SIZE, coords.y / REGION_SIZE, coords.z / REGION_SIZE);
        ConstrainedVec3<CHUNK_SIZE> chunkCoord(coords.x % CHUNK_SIZE, coords.y % CHUNK_SIZE, coords.z % CHUNK_SIZE);

        return getChunkBlock(regionCoord, chunkCoord, knownNeighbour);
    }
private:
    // --- CPU structs ---
    // Chunk coordinate - heavily optimized for RAM usage

    // Raw chunk - unoptimized
    template<size_t SIZE>
    struct RawChunk {
        std::array<uint16_t, SIZE * SIZE * SIZE> blockArray;
    };

    // PalletedChunk - optimized with a pallete, but can only hold 254 unique blocks
    template<size_t SIZE>
    struct PalletedChunk {
        std::array<uint16_t, 256> pallete;
        std::array<uint8_t, SIZE * SIZE * SIZE> blockArray;

        std::unique_ptr<RawChunk<SIZE>> toRaw() {
            std::unique_ptr<RawChunk<SIZE>> output(new RawChunk<SIZE>);
            for (size_t i = 0; const auto& value : this->blockArray) {
                output->blockArray[i] = pallete[value];
                ++i;
            }
            return output;
        }
    };

    // SparseChunk - heavily optimized, but only for chunks with a low block count
    template<size_t SIZE>
    struct SparseChunk {
        struct sparseEncoding {
            uint16_t blockID;
            std::vector<ConstrainedVec3<SIZE>> coordinates;
        };

        std::vector<sparseEncoding> blockArray;

        std::unique_ptr<PalletedChunk<SIZE>> toPalleted() {
            std::unique_ptr<PalletedChunk<SIZE>> output(new PalletedChunk<SIZE>);
            for (size_t index = 1; auto& value : blockArray) {
                output->pallete[index] = value.blockID;
                for (auto& value2 : value.coordinates) {
                    output->blockArray[value2.flatten()] = index;
                }
                ++index;
            }
        }
    };

    // --- GPU structs ---
    struct GPUCubeFace {
        float x, y, z;
        int orientation;

        void setOrientation(int x, int y, int z) {
            orientation = (x & 3) | ((y & 3) << 2) | ((z & 3) << 4);
        }
    };

    struct GPUDrawArraysIndirectCommand {
        uint32_t count;         // Number of vertices
        uint32_t instanceCount; // Number of quads to draw
        uint32_t first;         // Starting vertex
        uint32_t baseInstance;  // Material ID
    };

    // --- Variables ---
    ankerl::unordered_dense::map<
        ConstrainedVec3<REGION_SIZE>,
        RawChunk<CHUNK_SIZE>,
        ConstrainedVec3Hasher<REGION_SIZE>,
        ConstrainedVec3Equal<REGION_SIZE>
    >rawLocalRegion;
    ankerl::unordered_dense::map<
        ConstrainedVec3<REGION_SIZE>,
        PalletedChunk<CHUNK_SIZE>,
        ConstrainedVec3Hasher<REGION_SIZE>,
        ConstrainedVec3Equal<REGION_SIZE>
    > palletedLocalRegion;
    ankerl::unordered_dense::map<
        ConstrainedVec3<REGION_SIZE>,
        SparseChunk<CHUNK_SIZE>,
        ConstrainedVec3Hasher<REGION_SIZE>,
        ConstrainedVec3Equal<REGION_SIZE>
    > sparseLocalRegion;

    // --- Meshing functions ---
    void meshChunk(std::vector<GPUCubeFace>& faces, std::vector<GPUDrawArraysIndirectCommand>& cmd, ConstrainedVec3<REGION_SIZE> chunkCoord) {
        // Delete old data
        faces.empty();
        cmd.empty();

        // Stuff for finding neighbours
        typedef uint8_t mapHint;
        std::array<mapHint, 6> neighbourHints; // 1: top // 2: bottom // 3: north // 4: east // 5: south // 6: west

        // Get a copy of the chunk we are working on
        bool sparseChunk = false;
        PalletedChunk<CHUNK_SIZE> workingChunk;
        if (sparseLocalRegion.contains(chunkCoord)) {
            auto temp = sparseLocalRegion[chunkCoord].toPalleted();
            workingChunk = *temp;
        }
        if (palletedLocalRegion.contains(chunkCoord) || sparseChunk) {
            if (!sparseChunk)
                workingChunk = palletedLocalRegion[chunkCoord];

            // Loop through the array
            for (int x = 0; x < CHUNK_SIZE; x++) {
                for (int y = 0; y < CHUNK_SIZE; y++) {
                    for (int z = 0; z < CHUNK_SIZE; z++) {
                        std::bitset<6> existentialMatrix;

                        // Check top face
                        if (z + 1 < CHUNK_SIZE)
                            existentialMatrix[0] = static_cast<bool>(workingChunk[ConstrainedVec3<CHUNK_SIZE>(x, y, z + 1).flatten()]);
                        else {
                            existentialMatrix[0] = static_cast<bool>(getChunkBlock(ConstrainedVec3<CHUNK_SIZE>(chunkCoord.x, chunkCoord.y, chunkCoord.z + 1), ConstrainedVec3<CHUNK_SIZE>(x, y, 0)));
                        }

                        // Check bottom face
                        if (z - 1 >= 0)
                            existentialMatrix[1] = static_cast<bool>(workingChunk[ConstrainedVec3<CHUNK_SIZE>(x, y, z - 1).flatten()]);
                        else {
                            existentialMatrix[1] = static_cast<bool>(getChunkBlock(ConstrainedVec3<CHUNK_SIZE>(chunkCoord.x, chunkCoord.y, chunkCoord.z - 1), ConstrainedVec3<CHUNK_SIZE>(x, y, CHUNK_SIZE - 1)));
                        }

                        // Check north face
                        if (x + 1 < CHUNK_SIZE)
                            existentialMatrix[2] = static_cast<bool>(workingChunk[ConstrainedVec3<CHUNK_SIZE>(x + 1, y, z).flatten()]);
                        else {
                            existentialMatrix[2] = static_cast<bool>(getChunkBlock(ConstrainedVec3<CHUNK_SIZE>(chunkCoord.x + 1, chunkCoord.y, chunkCoord.z), ConstrainedVec3<CHUNK_SIZE>(0, y, z)));
                        }

                        // Check south face
                        if (x - 1 >= 0)
                            existentialMatrix[4] = static_cast<bool>(workingChunk[ConstrainedVec3<CHUNK_SIZE>(x - 1, y, z).flatten()]);
                        else {
                            existentialMatrix[4] = static_cast<bool>(getChunkBlock(ConstrainedVec3<CHUNK_SIZE>(chunkCoord.x - 1, chunkCoord.y, chunkCoord.z), ConstrainedVec3<CHUNK_SIZE>(CHUNK_SIZE - 1, y, z)));
                        }

                        // Check east face
                        if (y + 1 < CHUNK_SIZE)
                            existentialMatrix[3] = static_cast<bool>(workingChunk[ConstrainedVec3<CHUNK_SIZE>(x, y + 1, z).flatten()]);
                        else {
                            existentialMatrix[3] = static_cast<bool>(getChunkBlock(ConstrainedVec3<CHUNK_SIZE>(chunkCoord.x, chunkCoord.y + 1, chunkCoord.z), ConstrainedVec3<CHUNK_SIZE>(x, 0, z)));
                        }

                        // Check west face
                        if (y - 1 >= 0)
                            existentialMatrix[5] = static_cast<bool>(workingChunk[ConstrainedVec3<CHUNK_SIZE>(x, y - 1, z).flatten()]);
                        else {
                            existentialMatrix[5] = static_cast<bool>(getChunkBlock(ConstrainedVec3<CHUNK_SIZE>(chunkCoord.x, chunkCoord.y - 1, chunkCoord.z), ConstrainedVec3<CHUNK_SIZE>(x, CHUNK_SIZE - 1, z)));
                        }
                    }
                }
            }

        } else if (rawLocalRegion.contains(chunkCoord)) {
            spdlog::warn("Raw chunk meshing not implemented");
        } else {
            DEBUG( spdlog::warn("Tried to mesh missing chunk at X: {}; Y:{}; Z:{}", chunkCoord.x, chunkCoord.y, chunkCoord.z) );
        }

        // Shrink, just in case
        faces.shrink_to_fit();
        cmd.shrink_to_fit();
    }
};
