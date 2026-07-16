#include <algorithm> // IWYU pragma: keep
#include <utils/enums.hpp>
#include "config.hpp"

struct SparseChunk {
    struct sparseEncoding {
        uint16_t blockID=0;
        std::vector<chunkCoord> coordinates{};
    };

    std::vector<sparseEncoding> blockArray{};

    setBlockStatus setBlock(ConstrainedVec3<CHUNK_SIZE> coords, uint16_t blockID);
};
