#include "region.hpp"

Region::Region(BlocksManager* Manager) : Manager(Manager) {
    glCreateBuffers(1, &MDIcmdsSSBO);
    glGenBuffers(1, &seedsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, seedsSSBO);
}

Region::~Region() {
    glDeleteBuffers(1, &MDIcmdsSSBO);
    glDeleteBuffers(1, &seedsSSBO);
}

void Region::meshWorker(regionCoord regionCoords, std::vector<GPU::CubeFace> &faces, std::vector<GPU::DrawArraysIndirectCommand> &cmds) {
    std::array<std::array<bool, CHUNK_SIZE*CHUNK_SIZE>, 6> neighbourMasks = {};
    chunkType currentChunkType = chunkLookup[regionCoords.flatten()];
    if (currentChunkType == chunkType::Empty){
        DEBUG(spdlog::warn("Tried to mesh missing chunk at: ({}, {}, {}) - region coord",
            static_cast<int>( regionCoords.x), static_cast<int>(regionCoords.y), static_cast<int>(regionCoords.z) );
        );
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
