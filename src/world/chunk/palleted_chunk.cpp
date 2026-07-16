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
