#include "sparse_chunk.hpp"

// std::unique_ptr<PalletedChunk<SIZE>> toPalleted() {
//     auto output = std::make_unique<PalletedChunk<SIZE>>();
//     output->pallete[0] = 0;

//     for (size_t index = 1; auto& value : blockArray) {
//         output->pallete[index] = value.blockID;
//         for (auto& value2 : value.coordinates) {
//             output->blockArray[value2.flatten()] = index;
//         }
//         ++index;
//     }
//     return output;
// }

setBlockStatus SparseChunk::setBlock(chunkCoord coords, uint16_t blockID) {
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

std::array<bool, CHUNK_SIZE * CHUNK_SIZE> SparseChunk::faceMask(cardinalDirection dir) {
    std::array<bool, CHUNK_SIZE * CHUNK_SIZE> output = {};

    for (const auto& encoding : blockArray) {
        if (encoding.blockID == 0) {
            break;
        }
        for (const auto& coords : encoding.coordinates) {
            switch (dir) {
            case cardinalDirection::North:
                if (coords.z != 0) continue;
                output[coords.x + CHUNK_SIZE * coords.y] = true;
                break;
            case cardinalDirection::East:
                if (coords.x != CHUNK_SIZE) continue;
                output[coords.z + CHUNK_SIZE * coords.y] = true;
                break;
            case cardinalDirection::South:
                if (coords.z != CHUNK_SIZE) continue;
                output[coords.x + CHUNK_SIZE * coords.y] = true;
                break;
            case cardinalDirection::West:
                if (coords.x != 0) continue;
                output[coords.z + CHUNK_SIZE * coords.y] = true;
                break;
            case cardinalDirection::Up:
                if (coords.y != CHUNK_SIZE) continue;
                output[coords.x + CHUNK_SIZE * coords.z] = true;
                break;
            case cardinalDirection::Down:
                if (coords.y != 0) continue;
                output[coords.x + CHUNK_SIZE * coords.z] = true;
                break;
            }
        }
    }

    return output;
}
