// Logging
#include <spdlog/spdlog.h>

// I/O management
#include <SDL3/SDL.h>

// OpenGL
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
#include <vector>
#include "SDL3/SDL_video.h"
#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"
#include "glbinding/gl/types.h"

// Utils
#include "shader.hpp"
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
    SDL_Window* window = SDL_CreateWindow("Quarryfied", 1280, 720, SDL_WINDOW_OPENGL);
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

    // Dummy data
    CubeFace myFace;
    myFace.orientation = packCubeFaceOrientation(0, 0, 0);
    myFace.x = -0.5f;
    myFace.y = 0.0f;
    myFace.z = 0.0f;
    std::vector<CubeFace>seeds;
    seeds.push_back(myFace);

    // Create VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    // Create SSBO
    GLuint SSBO;
    glGenBuffers(1, &SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    glBufferData(gl::GLenum::GL_SHADER_STORAGE_BUFFER, seeds.size() * sizeof(CubeFace), seeds.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);

    // Main loop
    spdlog::info("Initialization finished! Entering main loop.");
    glClearColor(0.45f, 0.71f, 0.95f, 1.0f);
    bool running = true;
    while (running) {
        // Poll SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
        }

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Really cool rendering logic
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);
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
