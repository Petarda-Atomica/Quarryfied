// Logging
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_scancode.h"
#include "glbinding/gl/bitfield.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include <cmath>
#include <spdlog/spdlog.h>

// I/O management
#include <SDL3/SDL.h>

// OpenGL
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
#include "SDL3/SDL_video.h"
#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"
#include "glbinding/gl/types.h"

// Utils
#include "player.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "materials.hpp"
#include "blocks.hpp"
#include "chunk.hpp"
#include "timer.hpp"


using namespace gl;

// Function for handling OpenGL errors
DEBUG (
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
);

void windowResizeCallback(int new_width, int new_height, Camera* cam) {
    // Make sure the window isn't minimized or bugged
    if (new_height <= 0 || new_width <= 0) return;

    // Update viewport
    glViewport(0, 0, new_width, new_height);
    cam->setViewportSize(glm::vec2(new_width, new_height));
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
    DEBUG(
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    );

    // Make window
    spdlog::info("Creating window...");
    SDL_Window* window = SDL_CreateWindow("Quarryfied", 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    SDL_SetWindowRelativeMouseMode(window, true);

    // Init glbinding
    spdlog::info("Initializing glbinding...");
    glbinding::Binding::initialize([](const char* name) {
        return reinterpret_cast<glbinding::ProcAddress>(SDL_GL_GetProcAddress(name));
    }, false);

    // OpenGL debugging
    DEBUG (
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glErrCallback, nullptr);
    );

    // OpenGL settings
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    // Load shaders
    Shader mainShader("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
    mainShader.use();

    // Create VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    Player<> Steve;
    Steve.position.x = 3;
    Steve.position.y = 0;
    Steve.position.z = 3;
    Steve.lookAt(glm::vec3(0,0,0));

    // Create new Material Manager, upload data to it and bind it
    MaterialManager materials;

    BlocksManager aManager(&materials);
    aManager.loadBlocks("main");
    materials.bufferData();

    ChunkManager<16, 32> chunks(&aManager);
    chunks.setBlock(ChunkManager<16, 32>::ConstrainedVec3<16*32>(0,0,0), 1);
    chunks.setBlock(ChunkManager<16, 32>::ConstrainedVec3<16*32>(1,0,0), 1);
    chunks.setBlock(ChunkManager<16, 32>::ConstrainedVec3<16*32>(0,0,1), 1);
    chunks.setBlock(ChunkManager<16, 32>::ConstrainedVec3<16*32>(1,0,1), 1);
    chunks.setBlock(ChunkManager<16, 32>::ConstrainedVec3<16*32>(0,1,0), 1);
    auto drawingSize = chunks.renderChunk(ChunkManager<16, 32>::ConstrainedVec3<32>(0,0,0));

    // Main loop
    spdlog::info("Initialization finished! Entering main loop.");
    glClearColor(0.45f, 0.71f, 0.95f, 1.0f);
    HighResolutionTimer timer;
    bool running = true;
    while (running) {
        timer.Tick();

        // Poll SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                windowResizeCallback(event.window.data1, event.window.data2, Steve.camera.get());
                break;
            case SDL_EVENT_MOUSE_MOTION:
                float dx = event.motion.xrel;
                float dy = event.motion.yrel;

                Steve.position.yaw += dx * 0.5;
                Steve.position.pitch += dy * 0.5;

                // Clamp pitch
                if (Steve.position.pitch > +89.0f) Steve.position.pitch = +89.0f;
                if (Steve.position.pitch < -89.0f) Steve.position.pitch = -89.0f;
                break;
            }
        }

        // Poll keys used for movement
        const bool* keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_W]) {
            Steve.position.x += Steve.attributes.speed * timer.DeltaTime() * sin(glm::radians(Steve.position.yaw));
            Steve.position.z -= Steve.attributes.speed * timer.DeltaTime() * cos(glm::radians(Steve.position.yaw));
        }
        if (keys[SDL_SCANCODE_S]) {
            Steve.position.x -= Steve.attributes.speed * timer.DeltaTime() * sin(glm::radians(Steve.position.yaw));
            Steve.position.z += Steve.attributes.speed * timer.DeltaTime() * cos(glm::radians(Steve.position.yaw));
        }
        if (keys[SDL_SCANCODE_A]) {
            Steve.position.x -= Steve.attributes.speed * timer.DeltaTime() * cos(glm::radians(Steve.position.yaw));
            Steve.position.z -= Steve.attributes.speed * timer.DeltaTime() * sin(glm::radians(Steve.position.yaw));
        }
        if (keys[SDL_SCANCODE_D]) {
            Steve.position.x += Steve.attributes.speed * timer.DeltaTime() * cos(glm::radians(Steve.position.yaw));
            Steve.position.z += Steve.attributes.speed * timer.DeltaTime() * sin(glm::radians(Steve.position.yaw));
        }
        if (keys[SDL_SCANCODE_SPACE]) {
            Steve.position.y += Steve.attributes.speed * timer.DeltaTime();
        }
        if (keys[SDL_SCANCODE_LSHIFT]) {
            Steve.position.y -= Steve.attributes.speed * timer.DeltaTime();
        }

        Steve.syncCameras();

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Bind stuff
        Steve.camera->bind(2);
        materials.bind(1);

        // Really cool rendering logic
        glBindVertexArray(VAO);
        chunks.draw(drawingSize);
        // ------------------------ //

        // Swap
        SDL_GL_SwapWindow(window);
    }

    spdlog::info("Doing cleanup...");
    Steve.destroy();
    materials.destroy();
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
