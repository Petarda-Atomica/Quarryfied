#pragma once

#include <utils/enums.hpp>
#include <cstdint>

namespace GPU {
    struct DrawArraysIndirectCommand {
        uint32_t count;         // Number of vertices
        uint32_t instanceCount; // Number of quads to draw
        uint32_t first;         // Starting vertex
        uint32_t baseInstance;  // Material ID
    };

    struct CubeFace {
        float x, y, z;
        int orientation;

        void setOrientation(int x, int y, int z) {
            orientation = (x & 3) | ((y & 3) << 2) | ((z & 3) << 4);
        }

        void setOrientation(cardinalDirection dir) {
            switch (dir) {
                case cardinalDirection::North: setOrientation(2, 0, 0); break;
                case cardinalDirection::East:  setOrientation(0, 1, 0); break;
                case cardinalDirection::South: setOrientation(0, 0, 0); break;
                case cardinalDirection::West:  setOrientation(0, 3, 0); break;
                case cardinalDirection::Up:    setOrientation(3, 0, 0); break;
                case cardinalDirection::Down:  setOrientation(1, 0, 0); break;
            }
        }
    };
}
