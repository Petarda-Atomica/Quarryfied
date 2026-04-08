#include <SDL3/SDL.h>

// This is the "Everything" header for glbinding plumbing
#include <glbinding/glbinding.h>

// This provides the actual OpenGL functions (gl::glClear, etc)
#include <glbinding/gl/gl.h>

// If the above still fails, some versions require this specific plumbing:
#include <glbinding/Binding.h>

using namespace gl;

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Quarryfied", 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    // Initialize glbinding (This should now be recognized)
    glbinding::Binding::initialize([](const char* name) {
        return reinterpret_cast<glbinding::ProcAddress>(SDL_GL_GetProcAddress(name));
    }, false);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
        }

        glClearColor(0.45f, 0.71f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
    }

    // --- FIX 2: Use the new SDL3 name ---
    SDL_GL_DestroyContext(glContext); // Renamed from DeleteContext

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
