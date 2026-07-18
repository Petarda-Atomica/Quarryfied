#include "raw_chunk.hpp"

setBlockStatus RawChunk::setBlock(chunkCoord coords, uint16_t blockID) {
    blockArray[coords.flatten()] = blockID;
    return setBlockStatus::Ok; // Everything is all right
}

std::array<bool, CHUNK_SIZE * CHUNK_SIZE> RawChunk::faceMask(cardinalDirection dir) {
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

void RawChunk::mesh(std::vector<GPU::CubeFace>& faces, std::vector<GPU::DrawArraysIndirectCommand>& cmds, regionCoord regionCoord, std::array<std::array<bool, CHUNK_SIZE * CHUNK_SIZE>, 6> neighboursFaceMask, BlocksManager* blocksManager) {
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
                auto textures = blocksManager->getTextures(blockArray[chunkCoord(x, y, z).flatten()]);

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
                    face.x = regionCoord.x + x;
                    face.y = regionCoord.y + y;
                    face.z = regionCoord.z + z - 0.5;
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
                    face.x = regionCoord.x + x + 0.5;
                    face.y = regionCoord.y + y;
                    face.z = regionCoord.z + z;
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
                    face.x = regionCoord.x + x;
                    face.y = regionCoord.y + y;
                    face.z = regionCoord.z + z + 0.5;
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
                    face.x = regionCoord.x + x - 0.5;
                    face.y = regionCoord.y + y;
                    face.z = regionCoord.z + z;
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
                    face.x = regionCoord.x + x;
                    face.y = regionCoord.y + y + 0.5;
                    face.z = regionCoord.z + z;
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
                    face.x = regionCoord.x + x;
                    face.y = regionCoord.y + y - 0.5;
                    face.z = regionCoord.z + z;
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
