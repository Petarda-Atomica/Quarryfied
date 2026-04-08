// Logging
#include <spdlog/spdlog.h>

// I/O management
#include <SDL3/SDL.h>

// OpenGL
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
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
