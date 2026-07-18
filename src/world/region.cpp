#include "region.hpp"
#include "spdlog/spdlog.h"
#include "utils/enums.hpp"
#include "world/chunk/config.hpp"

Region::Region(BlocksManager* Manager) : Manager(Manager) {
    glCreateBuffers(1, &MDIcmdsSSBO);
    glGenBuffers(1, &seedsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, seedsSSBO);
}

Region::~Region() {
    glDeleteBuffers(1, &MDIcmdsSSBO);
    glDeleteBuffers(1, &seedsSSBO);
}

setBlockStatus Region::setBlock(localizedRegionCoord coords, uint16_t blockID) {
    auto regionCoords = regionCoord(coords.x / REGION_SIZE, coords.y / REGION_SIZE, coords.z / REGION_SIZE);
    auto chunkCoords = chunkCoord(coords.x % REGION_SIZE, coords.y % REGION_SIZE, coords.z % REGION_SIZE);

    switch (chunkLookup[regionCoords.flatten()]) {
        case chunkType::Raw:        return rawRegion[regionCoords].setBlock(chunkCoords, blockID);
        case chunkType::Palleted:   return palletedRegion[regionCoords].setBlock(chunkCoords, blockID);
        case chunkType::Sparse:     return sparseRegion[regionCoords].setBlock(chunkCoords, blockID);
        case chunkType::Empty:
            DEBUG(spdlog::warn("No chunk at ({}, {}, {}). Creating a sparse chunk there",
                static_cast<int>(regionCoords.x), static_cast<int>(regionCoords.y), static_cast<int>(regionCoords.z));
            );
            sparseRegion[regionCoords] = {};
            chunkLookup[regionCoords.flatten()] = chunkType::Sparse;
            return sparseRegion[regionCoords].setBlock(chunkCoords, blockID);
    }
}

void Region::meshWorker(regionCoord regionCoords, std::vector<GPU::CubeFace> &faces, std::vector<GPU::DrawArraysIndirectCommand> &cmds) {
    std::array<std::array<bool, CHUNK_SIZE*CHUNK_SIZE>, 6> neighbourMasks = {};
    chunkType currentChunkType = chunkLookup[regionCoords.flatten()];
    if (currentChunkType == chunkType::Empty){
        // DEBUG(spdlog::warn("Tried to mesh missing chunk at: ({}, {}, {}) - region coord",
        //     static_cast<int>( regionCoords.x), static_cast<int>(regionCoords.y), static_cast<int>(regionCoords.z) );
        // );
        return;
    }

    // Lambda for getting neighbour masks
    auto getNeighbourMask = [&](cardinalDirection dir) {
        regionCoord nCoords = regionCoords;
        switch (dir) {
            case cardinalDirection::North:  nCoords.z -= 1; break;
            case cardinalDirection::East:   nCoords.x += 1; break;
            case cardinalDirection::South:  nCoords.z += 1; break;
            case cardinalDirection::West:   nCoords.x -= 1; break;
            case cardinalDirection::Up:     nCoords.y += 1; break;
            case cardinalDirection::Down:   nCoords.y -= 1; break;
        }

        chunkType nChunkType = chunkLookup[nCoords.flatten()];
        if (nChunkType == chunkType::Empty) return;

        auto& targetMask = neighbourMasks[static_cast<int>(dir)];

        switch (nChunkType) {
            case chunkType::Raw:      targetMask = rawRegion[nCoords].faceMask(dir); break;
            case chunkType::Palleted: targetMask = palletedRegion[nCoords].faceMask(dir); break;
            case chunkType::Sparse:   targetMask = sparseRegion[nCoords].faceMask(dir); break;
            default: break;
        }
    };

    // Get masks
    getNeighbourMask(cardinalDirection::North);
    getNeighbourMask(cardinalDirection::East);
    getNeighbourMask(cardinalDirection::South);
    getNeighbourMask(cardinalDirection::West);
    getNeighbourMask(cardinalDirection::Up);
    getNeighbourMask(cardinalDirection::Down);

    // Mesh chunk
    switch (currentChunkType) {
        case chunkType::Raw:        rawRegion[regionCoords].mesh(faces, cmds, regionCoords, neighbourMasks, Manager);       break;
        case chunkType::Palleted:   palletedRegion[regionCoords].mesh(faces, cmds, regionCoords, neighbourMasks, Manager);  break;
        case chunkType::Sparse:     sparseRegion[regionCoords].mesh(faces, cmds, regionCoords, neighbourMasks, Manager);    break;
        case chunkType::Empty:                                                                                              break;
    }
}

size_t Region::renderAroundChunk(regionCoord chunkCoords, int_fast8_t renderDistance, int_fast8_t verticalRenderDistance) {
    // Worker output struct
    struct WorkerOutput {
        std::vector<GPU::CubeFace> faces;
        std::vector<GPU::DrawArraysIndirectCommand> cmds;
    };

    std::vector<regionCoord> validCoords;

    // Find all the chunks that need meshing
    for (int x = -renderDistance; x < renderDistance; x++) {
        for (int z = -renderDistance; z < renderDistance; z++) {
            // Check that we are still in the region
            if (x + static_cast<int>(chunkCoords.x) < 0 || z + static_cast<int>(chunkCoords.y) < 0) continue;

            // Check if we are inside the circular render distance
            if (sqrt(static_cast<float>(x*x + z*z)) > static_cast<float>(renderDistance)) continue;

            // Do all the verticals of a valid chunk
            for (int y = -verticalRenderDistance; y < verticalRenderDistance; y++) {
                validCoords.push_back(regionCoord(x + chunkCoords.x, y + chunkCoords.y, z + chunkCoords.z));
            }
        }
    }
    if (validCoords.empty()) return 0;

    // Make the worker pool
    BS::thread_pool pool;
    std::vector<WorkerOutput> workerResults(validCoords.size());

    // Give the workers work to do
    for (size_t i = 0; i < validCoords.size(); ++i) {
        pool.detach_task([this, coord = validCoords[i], &res = workerResults[i]]() {
           this->meshWorker(coord, res.faces, res.cmds);
        });
    }

    // Wait for the workers to finish meshing
    pool.wait();

    // Merge vectors
    size_t totalFaces = 0;
    size_t totalCmds = 0;
    for (const auto& res : workerResults) {
        totalFaces += res.faces.size();
        totalCmds += res.cmds.size();
    }
    std::vector<GPU::CubeFace> finalFaces;
    std::vector<GPU::DrawArraysIndirectCommand> finalCmds;
    finalFaces.reserve(totalFaces);
    finalCmds.reserve(totalCmds);
    for (auto& res : workerResults) {
        finalFaces.insert(finalFaces.end(), std::make_move_iterator(res.faces.begin()), std::make_move_iterator(res.faces.end()));
        finalCmds.insert(finalCmds.end(), std::make_move_iterator(res.cmds.begin()), std::make_move_iterator(res.cmds.end()));
    }

    // Upload the data to the GPU
    glNamedBufferData(seedsSSBO, finalFaces.size() * sizeof(GPU::CubeFace), finalFaces.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, seedsSSBO);
    glNamedBufferStorage(MDIcmdsSSBO, finalCmds.size() * sizeof(GPU::DrawArraysIndirectCommand), finalCmds.data(), GL_DYNAMIC_STORAGE_BIT);

    return finalCmds.size();
}
