# CMake project config
CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
PROJECT(Quarryfied VERSION 0.0.1 LANGUAGES C CXX)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
INCLUDE(InteliSense)

# Make the executable
FILE(GLOB_RECURSE SRC_FILES "src/*.cpp" "src/*.c")
FILE(GLOB_RECURSE HDR_FILES "src/*.hpp" "src/*.h")
ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_FILES} ${HDR_FILES})

# Include local external libraries
FILE(GLOB EXTERN_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "extern/*")
TARGET_INCLUDE_DIRECTORIES(
  ${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/extern
)
foreach(DIR ${EXTERN_DIRS})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DIR} AND
    EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${DIR}/CMakeLists.txt)

    MESSAGE(STATUS "Configuring local external dependency: ${DIR}")
    ADD_SUBDIRECTORY(${DIR})

  endif()
endforeach()

# Include external dependencies
INCLUDE(Dependencies)
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
  PRIVATE
    SDL3::SDL3
    glbinding::glbinding
    glm::glm
    spdlog::spdlog
    lua_lib
    ryml::ryml
    unordered_dense
)

# Include LuaBridge3
TARGET_INCLUDE_DIRECTORIES(
  ${PROJECT_NAME} PRIVATE
  ${luabridge_SOURCE_DIR}/Source
)

# Copy the assets folder to the build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets to build directory..."
)

# Copy the game_resources folder to the build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/game_resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/game_resources"
    COMMENT "Copying game_resources to build directory..."
)

# Stuff to please IDEs
SOURCE_GROUP(TREE
  "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES
  ${SRC_FILES}
  ${HDR_FILES}
)

# Debugging macros
SET(MACRO_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/src/globalMacros.hpp")
IF(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE "/FI${MACRO_HEADER}")
ELSE()
  target_compile_options(${PROJECT_NAME} PRIVATE "-include" "${MACRO_HEADER}")
ENDIF()
