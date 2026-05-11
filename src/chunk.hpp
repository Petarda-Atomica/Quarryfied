#pragma once

#include "blocks.hpp"
#include "glbinding/gl/bitfield.h"
#include "glbinding/gl/types.h"
#include "spdlog/spdlog.h"
#include <algorithm>
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
#include <algorithm>

template<size_t CHUNK_SIZE, size_t REGION_SIZE>
class ChunkManager {
public:
    BlocksManager* blocksManager;
    ChunkManager (BlocksManager* blocksManager) : blocksManager(blocksManager) {
        glCreateBuffers(1, &MDIcmdsSSBO);
        glGenBuffers(1, &seedsSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, seedsSSBO);
    }

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

        // --- Operators ---
        bool operator==(const ConstrainedVec3& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
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
                return {rawLocalRegion[regionCoord].blockArray[chunkCoord.flatten()], 1};
                break;
            case 2: {
                auto& chunk = palletedLocalRegion[regionCoord];
                auto blockID = chunk.blockArray[chunkCoord.flatten()];
                return {chunk.pallete[blockID], 2};
            }
            case 3: {
                auto& chunk = sparseLocalRegion[regionCoord];
                for (auto& encoding : chunk.blockArray) {
                    auto it = std::find(
                      encoding.coordinates.begin(),
                      encoding.coordinates.end(),
                      chunkCoord
                    );

                    if (it != encoding.coordinates.end())
                        return { encoding.blockID, 3 };
                }
            }
            }
        } else {
            if (rawLocalRegion.contains(regionCoord))
                return {rawLocalRegion[regionCoord].blockArray[chunkCoord.flatten()], 1};
            else if (palletedLocalRegion.contains(regionCoord))
                return getChunkBlock(regionCoord, chunkCoord, 2);
            else if (sparseLocalRegion.contains(regionCoord))
                return getChunkBlock(regionCoord, chunkCoord, 3);
        }

        return { 0, 0 };
    }

    std::pair<uint16_t, uint8_t> getBlock(ConstrainedVec3<CHUNK_SIZE * REGION_SIZE> coords, uint8_t knownNeighbour = 0) {
        ConstrainedVec3<REGION_SIZE> regionCoord(coords.x / REGION_SIZE, coords.y / REGION_SIZE, coords.z / REGION_SIZE);
        ConstrainedVec3<CHUNK_SIZE> chunkCoord(coords.x % CHUNK_SIZE, coords.y % CHUNK_SIZE, coords.z % CHUNK_SIZE);

        return getChunkBlock(regionCoord, chunkCoord, knownNeighbour);
    }

    void setBlock(ConstrainedVec3<CHUNK_SIZE * REGION_SIZE> coords, uint16_t blockID) {
        auto regionCoord = ConstrainedVec3<REGION_SIZE>(coords.x / CHUNK_SIZE, coords.y / CHUNK_SIZE, coords.z / CHUNK_SIZE);
        auto chunkCoord = ConstrainedVec3<CHUNK_SIZE>(coords.x % CHUNK_SIZE, coords.y % CHUNK_SIZE, coords.z % CHUNK_SIZE);

        if (rawLocalRegion.contains(regionCoord)) rawLocalRegion[regionCoord].setBlock(chunkCoord, blockID);
        else if (palletedLocalRegion.contains(regionCoord)) palletedLocalRegion[regionCoord].setBlock(chunkCoord, blockID);
        else if (sparseLocalRegion.contains(regionCoord)) sparseLocalRegion[regionCoord].setBlock(chunkCoord, blockID);
        else {
            sparseLocalRegion[regionCoord] = {};
            sparseLocalRegion[regionCoord].setBlock(chunkCoord, blockID);
        }
    }

    // TEMP CODE!!!
    int renderChunk(ConstrainedVec3<REGION_SIZE> coord) {
        std::vector<GPUCubeFace> faces;
        std::vector<GPUDrawArraysIndirectCommand> cmds;
        meshChunk(faces, cmds, coord);

        // TEMP CODE!!! - load seeds
        glBufferData(GL_SHADER_STORAGE_BUFFER, faces.size() * sizeof(GPUCubeFace), faces.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, seedsSSBO);

        // TEMP CODE!!! - load MDI data
        glNamedBufferStorage(MDIcmdsSSBO, cmds.size() * sizeof(GPUDrawArraysIndirectCommand), cmds.data(), GL_DYNAMIC_STORAGE_BIT);

        return cmds.size();
    }

    void draw(GLsizei drawingSize) {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, MDIcmdsSSBO);
        glMultiDrawArraysIndirect(
            GL_TRIANGLE_STRIP,
            (void*)0,
            drawingSize,
            0
        );
    }

private:
    // --- Enums ---
    enum class setBlockStatus {
        Ok = 0,
        NothingToDo = 1,
        NeedsPromotion = 2,
    };

    // --- CPU structs ---
    // Chunk coordinate - heavily optimized for RAM usage

    // Raw chunk - unoptimized
    template<size_t SIZE>
    struct RawChunk {
        std::array<uint16_t, SIZE * SIZE * SIZE> blockArray;

        setBlockStatus setBlock(ConstrainedVec3<CHUNK_SIZE> coords, uint16_t blockID) {
            blockArray[coords.flatten()] = blockID;
            return setBlockStatus::Ok; // Everything is all right
        }
    };

    // PalletedChunk - optimized with a pallete, but can only hold 255 unique blocks
    template<size_t SIZE>
    struct PalletedChunk {
        std::array<uint16_t, 255> pallete{};
        std::array<uint8_t, SIZE * SIZE * SIZE> blockArray{};

        std::unique_ptr<RawChunk<SIZE>> toRaw() {
            std::unique_ptr<RawChunk<SIZE>> output(new RawChunk<SIZE>);
            for (size_t i = 0; const auto& value : this->blockArray) {
                output->blockArray[i] = pallete[value];
                ++i;
            }
            return output;
        }

        inline uint8_t getPalletedBlockAt(ConstrainedVec3<CHUNK_SIZE> chunkCoord) {
            return blockArray[chunkCoord.flatten()];
        }

        inline uint16_t getBlockAt(ConstrainedVec3<CHUNK_SIZE> chunkCoord) {
            return pallete[this->getPalletedBlockAt(chunkCoord)];
        }

        setBlockStatus setBlock(ConstrainedVec3<CHUNK_SIZE> coords, uint16_t blockID) {
            if (blockID == 0) {
                blockArray[coords.flatten()] = 0;
                return setBlockStatus::Ok;
            }
            for (int i = 1; i <= 255; ++i) {
                if (pallete[i] == blockID) {
                    blockArray[coords.flatten()] = static_cast<uint8_t>(i);
                    return setBlockStatus::Ok;
                }

                if (pallete[i] == 0) {
                    pallete[i] = blockID;
                    blockArray[coords.flatten()] = static_cast<uint8_t>(i);
                    return setBlockStatus::Ok;
                }
            }

            return setBlockStatus::NeedsPromotion; // Requires promotion
        }
    };

    // SparseChunk - heavily optimized, but only for chunks with a low block count
    template<size_t SIZE>
    struct SparseChunk {
        struct sparseEncoding {
            uint16_t blockID=0;
            std::vector<ConstrainedVec3<SIZE>> coordinates{};
        };

        std::vector<sparseEncoding> blockArray{};

        std::unique_ptr<PalletedChunk<SIZE>> toPalleted() {
            auto output = std::make_unique<PalletedChunk<SIZE>>();
            output->pallete[0] = 0;

            for (size_t index = 1; auto& value : blockArray) {
                output->pallete[index] = value.blockID;
                for (auto& value2 : value.coordinates) {
                    output->blockArray[value2.flatten()] = index;
                }
                ++index;
            }
            return output;
        }

        setBlockStatus setBlock(ConstrainedVec3<CHUNK_SIZE> coords, uint16_t blockID) {
            bool setSuccess = false;
            for (auto& encoding : blockArray) {
                if (encoding.blockID == blockID) {
                    // Make sure we don't already have this block set
                    auto it = std::find(encoding.coordinates.begin(), encoding.coordinates.end(), coords);
                    if (it != encoding.coordinates.end()) {
                        return setBlockStatus::NothingToDo;
                    }
                    // Set the block
                    encoding.coordinates.push_back(coords);
                    setSuccess = true;
                } else {
                    // Delete if we find another block at this position
                    auto it = std::find(encoding.coordinates.begin(), encoding.coordinates.end(), coords);
                    if (it != encoding.coordinates.end()) {
                        encoding.coordinates.erase(it);
                    }
                }
            }
            if (!setSuccess) {
                sparseEncoding temp;
                temp.blockID = blockID;
                temp.coordinates.emplace_back(coords);
                blockArray.emplace_back(temp);
            }

            return setBlockStatus::Ok;
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

    GLuint MDIcmdsSSBO;
    GLuint seedsSSBO;

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
    void meshChunk(std::vector<GPUCubeFace>& faces, std::vector<GPUDrawArraysIndirectCommand>& cmds, ConstrainedVec3<REGION_SIZE> regionCoord) {
        // Delete old data
        faces.clear();
        cmds.clear();

        // Stuff for finding neighbours
        // TODO: Implement a use for this
        typedef uint8_t mapHint;
        std::array<mapHint, 6> neighbourHints; // 1: top // 2: bottom // 3: north // 4: east // 5: south // 6: west

        // Get a copy of the chunk we are working on
        bool sparseChunk = false;
        std::unique_ptr<PalletedChunk<CHUNK_SIZE>> tempStorage;
        PalletedChunk<CHUNK_SIZE>* workingChunkPtr = nullptr;
        if (sparseLocalRegion.contains(regionCoord)) {
            tempStorage = sparseLocalRegion[regionCoord].toPalleted();
            workingChunkPtr = tempStorage.get();
            sparseChunk = true;
        }
        if (palletedLocalRegion.contains(regionCoord) || sparseChunk) {
            if (!sparseChunk)
                workingChunkPtr = &palletedLocalRegion[regionCoord];

            // Loop through the array
            ankerl::unordered_dense::map<uint16_t, std::vector<GPUCubeFace>> drawMap;
            for (int x = 0; x < CHUNK_SIZE; x++) {
                for (int y = 0; y < CHUNK_SIZE; y++) {
                    for (int z = 0; z < CHUNK_SIZE; z++) {

                        // Get the textures of the block we are working with
                        auto blockID =  workingChunkPtr->getBlockAt(ConstrainedVec3<CHUNK_SIZE>(x, y, z));
                        // if air, stop
                        if (blockID == 0) continue;
                        auto textures = blocksManager->getTextures(blockID);

                        std::bitset<6> existentialMatrix;
                        // Check top face
                        if (z + 1 < CHUNK_SIZE) {
                            existentialMatrix[0] = !static_cast<bool>( workingChunkPtr->getPalletedBlockAt(ConstrainedVec3<CHUNK_SIZE>(x, y, z + 1)) );}
                        else {
                            existentialMatrix[0] = !static_cast<bool>(getChunkBlock(ConstrainedVec3<REGION_SIZE>(regionCoord.x, regionCoord.y, regionCoord.z + 1), ConstrainedVec3<CHUNK_SIZE>(x, y, 0)).first);
                        }

                        // Check bottom face
                        if (z - 1 >= 0)
                            existentialMatrix[1] = !static_cast<bool>( workingChunkPtr->getPalletedBlockAt(ConstrainedVec3<CHUNK_SIZE>(x, y, z - 1)) );
                        else {
                            existentialMatrix[1] = !static_cast<bool>(getChunkBlock(ConstrainedVec3<REGION_SIZE>(regionCoord.x, regionCoord.y, regionCoord.z - 1), ConstrainedVec3<CHUNK_SIZE>(x, y, CHUNK_SIZE - 1)).first);
                        }

                        // Check north face
                        if (x + 1 < CHUNK_SIZE)
                            existentialMatrix[2] = !static_cast<bool>( workingChunkPtr->getPalletedBlockAt(ConstrainedVec3<CHUNK_SIZE>(x + 1, y, z)) );
                        else {
                            existentialMatrix[2] = !static_cast<bool>(getChunkBlock(ConstrainedVec3<REGION_SIZE>(regionCoord.x + 1, regionCoord.y, regionCoord.z), ConstrainedVec3<CHUNK_SIZE>(0, y, z)).first);
                        }

                        // Check south face
                        if (x - 1 >= 0)
                            existentialMatrix[4] = !static_cast<bool>( workingChunkPtr->getPalletedBlockAt(ConstrainedVec3<CHUNK_SIZE>(x - 1, y, z)) );
                        else {
                            existentialMatrix[4] = !static_cast<bool>(getChunkBlock(ConstrainedVec3<REGION_SIZE>(regionCoord.x - 1, regionCoord.y, regionCoord.z), ConstrainedVec3<CHUNK_SIZE>(CHUNK_SIZE - 1, y, z)).first);
                        }

                        // Check east face
                        if (y + 1 < CHUNK_SIZE)
                            existentialMatrix[3] = !static_cast<bool>( workingChunkPtr->getPalletedBlockAt(ConstrainedVec3<CHUNK_SIZE>(x, y + 1, z)) );
                        else {
                            existentialMatrix[3] = !static_cast<bool>(getChunkBlock(ConstrainedVec3<REGION_SIZE>(regionCoord.x, regionCoord.y + 1, regionCoord.z), ConstrainedVec3<CHUNK_SIZE>(x, 0, z)).first);
                        }

                        // Check west face
                        if (y - 1 >= 0)
                            existentialMatrix[5] = !static_cast<bool>( workingChunkPtr->getPalletedBlockAt(ConstrainedVec3<CHUNK_SIZE>(x, y - 1, z)) );
                        else {
                            existentialMatrix[5] = !static_cast<bool>(getChunkBlock(ConstrainedVec3<REGION_SIZE>(regionCoord.x, regionCoord.y - 1, regionCoord.z), ConstrainedVec3<CHUNK_SIZE>(x, CHUNK_SIZE - 1, z)).first);
                        }

                        // Actual meshing
                        GPUCubeFace face;
                        face.x = regionCoord.x * CHUNK_SIZE + x;
                        face.y = regionCoord.y * CHUNK_SIZE + y;
                        face.z = regionCoord.z * CHUNK_SIZE + z;
                        if (existentialMatrix[0]) {
                            face.setOrientation(3, 0, 0);
                            drawMap[textures.top].emplace_back(face);
                        }
                        if (existentialMatrix[1]) {
                            face.setOrientation(1, 0, 0);
                            drawMap[textures.bottom].emplace_back(face);
                        }
                        if (existentialMatrix[2]) {
                            face.setOrientation(2, 0, 0);
                            drawMap[textures.north].emplace_back(face);
                        }
                        if (existentialMatrix[3]) {
                            face.setOrientation(0, 1, 0);
                            drawMap[textures.east].emplace_back(face);
                        }
                        if (existentialMatrix[4]) {
                            face.setOrientation(0, 0, 0);
                            drawMap[textures.south].emplace_back(face);
                        }
                        if (existentialMatrix[5]) {
                            face.setOrientation(0, 3, 0);
                            drawMap[textures.west].emplace_back(face);
                        }
                    }
                }
            }

            // Pack into draw calls
            uint32_t vertexID = 0;
            for (auto& [key, val] : drawMap) {
                // Build MID call
                GPUDrawArraysIndirectCommand cmd;
                cmd.count = val.size() * 4;
                cmd.instanceCount = val.size();
                cmd.first = vertexID;
                cmd.baseInstance = key;
                vertexID += cmd.instanceCount-1;
                cmds.emplace_back(cmd);

                // Merge seeds
                faces.insert(faces.end(), std::make_move_iterator(val.begin()), std::make_move_iterator(val.end()));
            }

        } else if (rawLocalRegion.contains(regionCoord)) {
            spdlog::warn("Raw chunk meshing not implemented");
        } else {
            DEBUG( spdlog::warn("Tried to mesh missing chunk at X: {}; Y:{}; Z:{}",
                static_cast<uint16_t>(regionCoord.x),
                static_cast<uint16_t>(regionCoord.y),
                static_cast<uint16_t>(regionCoord.z))
            );
        }

        // Shrink, just in case
        faces.shrink_to_fit();
        cmds.shrink_to_fit();
    }
};
