// Struct for holding the data for each cube face
#include "glbinding/gl/types.h"
#include <cstdint>

using namespace gl;

struct CubeFace {
    float x, y, z;
    int orientation;
};

struct Material {
    GLuint64 textureHandle;
    char padding[8];
    float baseColor[4];
};

struct DrawArraysIndirectCommand {
    uint32_t count;         // Number of vertices
    uint32_t instanceCount; // Number of quads to draw
    uint32_t first;         // Starting vertex
    uint32_t baseInstance;  // Offset for gl_BaseInstance
};

int packCubeFaceOrientation(int x, int y, int z) {
    return (x & 3) | ((y & 3) << 2) | ((z & 3) << 4);
}
