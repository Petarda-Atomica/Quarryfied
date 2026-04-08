// Struct for holding the data for each cube face
#include "glbinding/gl/types.h"

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

int packCubeFaceOrientation(int x, int y, int z) {
    return (x & 3) | ((y & 3) << 2) | ((z & 3) << 4);
}
