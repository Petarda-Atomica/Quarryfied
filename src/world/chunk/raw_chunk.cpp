#include "raw_chunk.hpp"

setBlockStatus RawChunk::setBlock(chunkCoord coords, uint16_t blockID) {
    blockArray[coords.flatten()] = blockID;
    return setBlockStatus::Ok; // Everything is all right
}
