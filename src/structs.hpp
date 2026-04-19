#include <cstdint>
#include <glbinding/gl/types.h>

using namespace gl;

struct CubeFace {
    float x, y, z;
    int orientation;
};

struct DrawArraysIndirectCommand {
    uint32_t count;         // Number of vertices
    uint32_t instanceCount; // Number of quads to draw
    uint32_t first;         // Starting vertex
    uint32_t baseInstance;  // Material ID
};

int packCubeFaceOrientation(int x, int y, int z) {
    return (x & 3) | ((y & 3) << 2) | ((z & 3) << 4);
}
