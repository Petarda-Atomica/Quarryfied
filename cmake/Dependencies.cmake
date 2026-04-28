INCLUDE(FetchContent)

# --- SDL (Windowing) ---
FetchContent_Declare(
  SDL3
  GIT_REPOSITORY  https://github.com/libsdl-org/SDL.git
  GIT_TAG         main
)
SET(SDL_TESTS OFF  CACHE BOOL "" FORCE)
SET(SDL_INSTALL OFF CACHE BOOL "" FORCE)
SET(SDL_STATIC ON CACHE BOOL "" FORCE)
MESSAGE(STATUS "Configuring remote external dependency: SDL3")
FetchContent_MakeAvailable(SDL3)

# --- glbinding (OpenGL) ---
FetchContent_Declare(
  glbinding
  GIT_REPOSITORY  https://github.com/cginternals/glbinding.git
  GIT_TAG         v3.3.0
)
SET(OPTION_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
SET(OPTION_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(OPTION_BUILD_DOCS OFF CACHE BOOL "" FORCE)
MESSAGE(STATUS "Configuring remote external dependency: glbinding")
FetchContent_MakeAvailable(glbinding)

# --- GLM (Math) ---
FetchContent_Declare(
  GLM
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8
)
MESSAGE(STATUS "Configuring remote external dependency: GLM")
FetchContent_MakeAvailable(GLM)

# --- spdlog (Logging) ---
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY  https://github.com/gabime/spdlog.git
  GIT_TAG         v1.17.0
)
MESSAGE(STATUS "Configuring remote external dependency: spdlog")
FetchContent_MakeAvailable(spdlog)

# --- Lua ---
include(lua)

# --- LuaBridge3 (Lua) ---
FetchContent_Declare(
  luabridge
  GIT_REPOSITORY  https://github.com/kunitoki/LuaBridge3.git
  GIT_TAG         master
)
MESSAGE(STATUS "Configuring remote external dependency: LuaBridge3")
FetchContent_MakeAvailable(luabridge)

# --- RapidYAML ---
FetchContent_Declare(
  ryml
  GIT_REPOSITORY  https://github.com/biojppm/rapidyaml.git
  GIT_TAG         master
  GIT_SHALLOW     FALSE
)
MESSAGE(STATUS "Configuring remote external dependency: ryml")
FetchContent_MakeAvailable(ryml)
