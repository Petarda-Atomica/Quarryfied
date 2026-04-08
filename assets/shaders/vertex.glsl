#version 460 core

struct QuadSeed {
    vec3 center;
    int orientation;
};

layout(std430, binding = 0) readonly buffer QuadData {
    QuadSeed seeds[];
};

uniform mat4 uVP; // View-Projection Matrix

flat out uint drawID;
out vec2 uv;

// Function to build a 90-degree increment rotation matrix
mat3 getRotationMatrix(int steps, vec3 axis) {
    float angle = float(steps) * 1.57079632679; // steps * PI/2
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    // Standard Axis-Angle rotation matrix
    return mat3(
        oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s,
        oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s,
        oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c
    );
}

void main() {
    QuadSeed data = seeds[gl_InstanceID];

    // Define the 4 corners of the quad (Triangle Strip)
    vec3 pos = vec3(0.0);
    if (gl_VertexID == 0) pos = vec3(-0.5, -0.5, 0.0);
    if (gl_VertexID == 1) pos = vec3(0.5, -0.5, 0.0);
    if (gl_VertexID == 2) pos = vec3(-0.5, 0.5, 0.0);
    if (gl_VertexID == 3) pos = vec3(0.5, 0.5, 0.0);

    // Unpack orientations (assuming 2 bits per axis)
    int rotX = (data.orientation >> 0) & 3;
    int rotY = (data.orientation >> 2) & 3;
    int rotZ = (data.orientation >> 4) & 3;

    // Build and apply rotation
    // Note: Order matters (XYZ)
    mat3 R = getRotationMatrix(rotZ, vec3(0, 0, 1)) * getRotationMatrix(rotY, vec3(0, 1, 0)) * getRotationMatrix(rotX, vec3(1, 0, 0));

    pos = R * pos;

    // Final transform
    gl_Position = vec4(pos + data.center, 1.0);

    // Send material data
    drawID = gl_BaseInstance;
    if (gl_VertexID == 0) uv = vec2(0.0, 0.0);
    if (gl_VertexID == 1) uv = vec2(1.0, 0.0);
    if (gl_VertexID == 2) uv = vec2(0.0, 1.0);
    if (gl_VertexID == 3) uv = vec2(1.0, 1.0);
}
