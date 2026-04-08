// Struct for holding the data for each cube face
struct CubeFace {
    float x, y, z;
    int orientation;
};

int packCubeFaceOrientation(int x, int y, int z) {
    return (x & 3) | ((y & 3) << 2) | ((z & 3) << 4);
}
