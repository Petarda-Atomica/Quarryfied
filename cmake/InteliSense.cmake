# --- Automatic IntelliSense Setup ---
if(WIN32)
  # Windows Junction logic (No Admin needed)
  ADD_CUSTOM_TARGET(gen_compile_commands_link ALL
        COMMAND cmd /c mklink /J "${CMAKE_SOURCE_DIR}\\compile_commands.json" "${CMAKE_BINARY_DIR}\\compile_commands.json"
        COMMENT "Creating Windows Junction for IntelliSense"
        VERBATIM
    )
else()
  # Unix Symlink logic
  set(SOURCE_DATABASE "${CMAKE_BINARY_DIR}/compile_commands.json")
  set(TARGET_DATABASE "${CMAKE_SOURCE_DIR}/compile_commands.json")

  message(STATUS "Setting up IntelliSense symlink...")

  # Create the symlink immediately during configuration
  execute_process(
          COMMAND ${CMAKE_COMMAND} -E create_symlink "${SOURCE_DATABASE}" "${TARGET_DATABASE}"
          RESULT_VARIABLE link_result
      )

  if(NOT link_result EQUAL 0)
    message(WARNING "Failed to create IntelliSense symlink.")
  endif()
endif()
