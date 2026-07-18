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
                case cardinalDirection::North: setOrientation(0, 0, 1); break; // Z = 1
                case cardinalDirection::East:  setOrientation(1, 0, 0); break; // X = 1
                case cardinalDirection::South: setOrientation(0, 0, 3); break; // Z = 3
                case cardinalDirection::West:  setOrientation(3, 0, 0); break; // X = 3
                case cardinalDirection::Up:    setOrientation(0, 1, 0); break; // Y = 1
                case cardinalDirection::Down:  setOrientation(0, 3, 0); break; // Y = 3
            }
        }
    };
}
