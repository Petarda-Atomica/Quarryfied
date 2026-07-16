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
