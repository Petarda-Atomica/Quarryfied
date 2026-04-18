// Logging
#include "SDL3/SDL_events.h"
#include "glbinding/gl/bitfield.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include <spdlog/spdlog.h>

// I/O management
#include <SDL3/SDL.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// OpenGL
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
#include <vector>
#include "SDL3/SDL_video.h"
#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"
#include "glbinding/gl/types.h"
#include "glbinding/gl/functions-patches.h"

// Utils
#include "shader.hpp"
#include "camera.hpp"
#include "structs.hpp"


using namespace gl;

// Function for handling OpenGL errors
#ifndef NDEBUG
void glErrCallback(GLenum source, GLenum type, GLuint id,
                    GLenum severity, GLsizei length,
                    const GLchar* message, const void* userParam) {

    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
    switch(severity) {
    case GL_DEBUG_SEVERITY_LOW:
        spdlog::warn("OpenGL LOW severity error: {}", message);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        spdlog::warn("OpenGL MEDIUM severity error: {}", message);
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        spdlog::error("OpenGL HIGH severity error: {}", message);
        break;
    default:
        spdlog::warn("OpenGL unknown error: {}", message);
        break;
    }

}
#endif

void windowResizeCallback(int new_width, int new_height) {
    // Make sure the window isn't minimized or bugged
    if (new_height <= 0 || new_width <= 0) return;

    // Update viewport
    glViewport(0, 0, new_width, new_height);
}

int main(int argc, char* argv[]) {
    // Start
    spdlog::info("Starting Quarryfied...");

    // Init SDL3
    spdlog::info("Initializing SDL3...");
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        spdlog::critical("Failed to initialize SDL3");
        return 1;
    }

    // SDL Attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // SDL3 debugging
    #ifndef NDEBUG
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    #endif

    // Make window
    spdlog::info("Creating window...");
    SDL_Window* window = SDL_CreateWindow("Quarryfied", 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    // Init glbinding
    spdlog::info("Initializing glbinding...");
    glbinding::Binding::initialize([](const char* name) {
        return reinterpret_cast<glbinding::ProcAddress>(SDL_GL_GetProcAddress(name));
    }, false);

    // OpenGL debugging
    #ifndef NDEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glErrCallback, nullptr);
    #endif

    // OpenGL settings
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Load shaders
    Shader mainShader("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
    mainShader.use();

    // Dummy verticies1
    std::vector<CubeFace>seeds;
    CubeFace myFace;
    myFace.orientation = packCubeFaceOrientation(0, 0, 0);
    myFace.x = -0.5f;
    myFace.y = -0.5f;
    myFace.z = 0.0f;
    seeds.push_back(myFace);

    // Dummy verticies2
    myFace.orientation = packCubeFaceOrientation(0, 0, 0);
    myFace.x = 0.5f;
    myFace.y = -0.5f;
    myFace.z = 0.0f;
    seeds.push_back(myFace);

    // Dummy verticies3
    myFace.orientation = packCubeFaceOrientation(0, 0, 0);
    myFace.x = -0.5f;
    myFace.y = 0.5f;
    myFace.z = 0.0f;
    seeds.push_back(myFace);

    // Dummy verticies4
    myFace.orientation = packCubeFaceOrientation(0, 0, 0);
    myFace.x = 0.5f;
    myFace.y = 0.5f;
    myFace.z = 0.0f;
    seeds.push_back(myFace);

    // Dummy texture
    stbi_set_flip_vertically_on_load(true);
    int width, height, nch;
    unsigned char* imgData = stbi_load("assets/textures/grass.png", &width, &height, &nch, 4);
    GLuint textureID;
    glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
    glTextureStorage2D(textureID, 1, GL_RGBA8, width, height);
    glTextureSubImage2D(textureID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imgData);
    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GLuint64 handle = glGetTextureHandleARB(textureID);
    glMakeTextureHandleResidentARB(handle);
    stbi_image_free(imgData);

    // Dummy material
    Material myMaterial;
    myMaterial.textureHandle = handle;
    myMaterial.baseColor[0] = 1.0f;
    myMaterial.baseColor[1] = 1.0f;
    myMaterial.baseColor[2] = 1.0f;
    myMaterial.baseColor[3] = 1.0f;
    std::vector<Material>materials;
    materials.push_back(myMaterial);

    // Dummy MDI data
    std::vector<DrawArraysIndirectCommand>drawCommands;
    DrawArraysIndirectCommand cmd;
    cmd.count = 4;
    cmd.instanceCount = 4;
    cmd.first = 0;
    cmd.baseInstance = 0;
    drawCommands.push_back(cmd);

    // Upload MDI data
    GLuint cmdBuffer;
    glCreateBuffers(1, &cmdBuffer);
    glNamedBufferStorage(cmdBuffer, drawCommands.size() * sizeof(DrawArraysIndirectCommand), drawCommands.data(), GL_DYNAMIC_STORAGE_BIT);

    // Create VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    // Create seeds SSBO
    GLuint seedsSSBO;
    glGenBuffers(1, &seedsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, seedsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, seeds.size() * sizeof(CubeFace), seeds.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, seedsSSBO);

    // Create materials SSBO
    GLuint materialsSSBO;
    glGenBuffers(1, &materialsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, materials.size() * sizeof(Material), materials.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialsSSBO);

    Camera mainCamera(
        glm::vec3(4, 3, 3),
        glm::radians(45.0f),
        glm::vec2(1280, 720)
    );
    mainCamera.bind(2);

    // Main loop
    spdlog::info("Initialization finished! Entering main loop.");
    glClearColor(0.45f, 0.71f, 0.95f, 1.0f);
    bool running = true;
    while (running) {
        // Poll SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                windowResizeCallback(event.window.data1, event.window.data2);
                break;
            }
        }

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Really cool rendering logic
        glBindVertexArray(VAO);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cmdBuffer);
        glMultiDrawArraysIndirect(
            GL_TRIANGLE_STRIP,
            (void*)0,
            drawCommands.size(),
            0
        );
        // ------------------------ //

        // Swap
        SDL_GL_SwapWindow(window);
    }

    spdlog::info("Doing cleanup...");
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
