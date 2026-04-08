#version 460 core
#extension GL_ARB_bindless_texture : require

struct Material {
    sampler2D diffuseTex;
    vec4 baseColor;
};

layout(std430, binding = 1) readonly buffer MaterialData {
    Material materials[];
};

flat in uint drawID;
in vec2 uv;

out vec4 FragColor;

void main()
{
    sampler2D tex = materials[drawID].diffuseTex;

    FragColor = texture(tex, uv) * materials[drawID].baseColor;
}
