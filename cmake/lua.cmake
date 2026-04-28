include(FetchContent)

# Download the source immediately at config time
FetchContent_Declare(
  lua_source
  URL https://www.lua.org/ftp/lua-5.3.5.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP false
)
FetchContent_MakeAvailable(lua_source)

# Manually create a library from the downloaded files
file(GLOB LUA_SOURCES "${lua_source_SOURCE_DIR}/src/*.c")
# Remove lua.c and luac.c (those are for the standalone executables)
list(REMOVE_ITEM LUA_SOURCES
    "${lua_source_SOURCE_DIR}/src/lua.c"
    "${lua_source_SOURCE_DIR}/src/luac.c"
)

add_library(lua_lib STATIC ${LUA_SOURCES})
target_include_directories(lua_lib PUBLIC "${lua_source_SOURCE_DIR}/src")

# Map variables for luacpp
set(LUA_INCLUDE_DIR "${lua_source_SOURCE_DIR}/src" CACHE PATH "" FORCE)
set(LUA_LIBRARIES lua_lib CACHE FILEPATH "" FORCE)
set(Lua_FOUND TRUE CACHE BOOL "" FORCE)
