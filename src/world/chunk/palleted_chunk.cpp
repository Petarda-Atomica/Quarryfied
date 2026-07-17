#include "palleted_chunk.hpp"

// PalletedChunk::std::unique_ptr<RawChunk<SIZE>> toRaw() {
//     std::unique_ptr<RawChunk<SIZE>> output(new RawChunk<SIZE>);
//     for (size_t i = 0; const auto& value : this->blockArray) {
//         output->blockArray[i] = pallete[value];
//         ++i;
//     }
//     return output;
// }

setBlockStatus PalletedChunk::setBlock(chunkCoord coords, uint16_t blockID) {
    // If air, just set block to air
    if (blockID == 0) {
        blockArray[coords.flatten()] = 0;
        return setBlockStatus::Ok;
    }

    for (int i = 1; i <= 255; ++i) {
        // If block in pallete, just do normal set
        if (pallete[i] == blockID) {
            blockArray[coords.flatten()] = static_cast<uint8_t>(i);
            return setBlockStatus::Ok;
        }

        // If reached the end of populated pallete, populate a new entry in the pallete and do a normal set
        if (pallete[i] == 0) {
            pallete[i] = blockID;
            blockArray[coords.flatten()] = static_cast<uint8_t>(i);
            return setBlockStatus::Ok;
        }
    }

    // If block wasn't found in the pallete and there was no place to add a new item to the pallete, throw a needs promotion error
    return setBlockStatus::NeedsPromotion; // Requires promotion
}

std::array<bool, CHUNK_SIZE * CHUNK_SIZE> PalletedChunk::faceMask(cardinalDirection dir) {
    std::array<bool, CHUNK_SIZE * CHUNK_SIZE> output = {};

    switch (dir) {
    case cardinalDirection::North:
        for (int y = 0; y < CHUNK_SIZE; y++)
            for (int x = 0; x < CHUNK_SIZE; x++)
                output[x + CHUNK_SIZE * y] = static_cast<bool>(blockArray[chunkCoord(x, y, 0).flatten()]);
        break;

    case cardinalDirection::East:
        for (int y = 0; y < CHUNK_SIZE; y++)
            for (int z = 0; z < CHUNK_SIZE; z++)
                output[z + CHUNK_SIZE * y] = static_cast<bool>(blockArray[chunkCoord(CHUNK_SIZE, y, z).flatten()]);
        break;

    case cardinalDirection::South:
        for (int y = 0; y < CHUNK_SIZE; y++)
            for (int x = 0; x < CHUNK_SIZE; x++)
                output[x + CHUNK_SIZE * y] = static_cast<bool>(blockArray[chunkCoord(x, y, CHUNK_SIZE).flatten()]);
        break;

    case cardinalDirection::West:
        for (int y = 0; y < CHUNK_SIZE; y++)
            for (int z = 0; z < CHUNK_SIZE; z++)
                output[z + CHUNK_SIZE * y] = static_cast<bool>(blockArray[chunkCoord(0, y, z).flatten()]);
        break;

    case cardinalDirection::Up:
        for (int z = 0; z < CHUNK_SIZE; z++)
            for (int x = 0; x < CHUNK_SIZE; x++)
                output[x + CHUNK_SIZE * z] = static_cast<bool>(blockArray[chunkCoord(x, CHUNK_SIZE, z).flatten()]);
        break;

    case cardinalDirection::Down:
        for (int z = 0; z < CHUNK_SIZE; z++)
            for (int x = 0; x < CHUNK_SIZE; x++)
                output[x + CHUNK_SIZE * z] = static_cast<bool>(blockArray[chunkCoord(x, 0, z).flatten()]);
        break;

    }

    return output;
}

void PalletedChunk::mesh(std::vector<GPU::CubeFace>& faces, std::vector<GPU::DrawArraysIndirectCommand>& cmds, regionCoord regionCoord, std::array<std::array<bool, CHUNK_SIZE * CHUNK_SIZE>, 6> neighboursFaceMask, BlocksManager* blocksManager) {
    // Delete old data
    faces.clear();
    cmds.clear();

    // Make an array to hold what we are going to draw
    ankerl::unordered_dense::map<uint16_t, std::vector<GPU::CubeFace>> drawMap;

    // Loop through all blocks
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                // Get the textures of the block
                auto textures = blocksManager->getTextures(pallete[blockArray[chunkCoord(x, y, z).flatten()]]);

                // Mesh north face
                if ([this, x, y, z, neighboursFaceMask]() {
                    // Check if at the edge of a chunk and visible
                    if (z == 0) {
                        return !neighboursFaceMask[static_cast<int>(cardinalDirection::North)][x + CHUNK_SIZE * y];
                    }

                    // Check if in chunk and visible
                    return !static_cast<bool>(blockArray[chunkCoord(x, y, z-1).flatten()]);
                }()) {
                    GPU::CubeFace face;
                    face.x = regionCoord.x;
                    face.y = regionCoord.y;
                    face.z = regionCoord.z - 0.5;
                    face.setOrientation(cardinalDirection::North);
                    drawMap[textures.north].emplace_back(face);
                }

                // Mesh east face
                if ([this, x, y, z, neighboursFaceMask]() {
                    // Check if at the edge of a chunk and visible
                    if (x == CHUNK_SIZE) {
                        return !neighboursFaceMask[static_cast<int>(cardinalDirection::East)][z + CHUNK_SIZE * y];
                    }

                    // Check if in chunk and visible
                    return !static_cast<bool>(blockArray[chunkCoord(x+1, y, z).flatten()]);
                }()) {
                    GPU::CubeFace face;
                    face.x = regionCoord.x + 0.5;
                    face.y = regionCoord.y;
                    face.z = regionCoord.z;
                    face.setOrientation(cardinalDirection::East);
                    drawMap[textures.east].emplace_back(face);
                }

                // Mesh south face
                if ([this, x, y, z, neighboursFaceMask]() {
                    // Check if at the edge of a chunk and visible
                    if (z == CHUNK_SIZE) {
                        return !neighboursFaceMask[static_cast<int>(cardinalDirection::South)][x + CHUNK_SIZE * y];
                    }

                    // Check if in chunk and visible
                    return !static_cast<bool>(blockArray[chunkCoord(x, y, z+1).flatten()]);
                }()) {
                    GPU::CubeFace face;
                    face.x = regionCoord.x;
                    face.y = regionCoord.y;
                    face.z = regionCoord.z + 0.5;
                    face.setOrientation(cardinalDirection::South);
                    drawMap[textures.south].emplace_back(face);
                }

                // Mesh west face
                if ([this, x, y, z, neighboursFaceMask]() {
                    // Check if at the edge of a chunk and visible
                    if (x == 0) {
                        return !neighboursFaceMask[static_cast<int>(cardinalDirection::West)][z + CHUNK_SIZE * y];
                    }

                    // Check if in chunk and visible
                    return !static_cast<bool>(blockArray[chunkCoord(x-1, y, z).flatten()]);
                }()) {
                    GPU::CubeFace face;
                    face.x = regionCoord.x - 0.5;
                    face.y = regionCoord.y;
                    face.z = regionCoord.z;
                    face.setOrientation(cardinalDirection::West);
                    drawMap[textures.west].emplace_back(face);
                }

                // Mesh top face
                if ([this, x, y, z, neighboursFaceMask]() {
                    // Check if at the edge of a chunk and visible
                    if (y == CHUNK_SIZE) {
                        return !neighboursFaceMask[static_cast<int>(cardinalDirection::Up)][x + CHUNK_SIZE * z];
                    }

                    // Check if in chunk and visible
                    return !static_cast<bool>(blockArray[chunkCoord(x, y+1, z).flatten()]);
                }()) {
                    GPU::CubeFace face;
                    face.x = regionCoord.x;
                    face.y = regionCoord.y + 0.5;
                    face.z = regionCoord.z;
                    face.setOrientation(cardinalDirection::Up);
                    drawMap[textures.top].emplace_back(face);
                }

                // Mesh bottom face
                if ([this, x, y, z, neighboursFaceMask]() {
                    // Check if at the edge of a chunk and visible
                    if (y == 0) {
                        return !neighboursFaceMask[static_cast<int>(cardinalDirection::Down)][x + CHUNK_SIZE * z];
                    }

                    // Check if in chunk and visible
                    return !static_cast<bool>(blockArray[chunkCoord(x, y-1, z).flatten()]);
                }()) {
                    GPU::CubeFace face;
                    face.x = regionCoord.x;
                    face.y = regionCoord.y - 0.5;
                    face.z = regionCoord.z;
                    face.setOrientation(cardinalDirection::Down);
                    drawMap[textures.bottom].emplace_back(face);
                }
            }
        }
    }

    // Pack into draw calls
    uint32_t instanceOffset = 0;
    for (auto& [textureID, val] : drawMap) {
        GPU::DrawArraysIndirectCommand cmd;
        cmd.count = 4;
        cmd.instanceCount = (uint32_t)val.size();
        cmd.first = 0;
        cmd.baseInstance = instanceOffset;
        cmds.emplace_back(cmd);

        instanceOffset += (uint32_t)val.size();

        faces.insert(faces.end(), std::make_move_iterator(val.begin()), std::make_move_iterator(val.end()));
    }

    // Shrink, just in case
    faces.shrink_to_fit();
    cmds.shrink_to_fit();
}
