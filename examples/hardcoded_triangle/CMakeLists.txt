add_executable(
    hardcoded_triangle
    src/main.cpp
)
target_link_libraries(
    hardcoded_triangle PRIVATE engine
)

message(STATUS "config $<CONFIG>")

add_custom_command(
    TARGET hardcoded_triangle POST_BUILD
    COMMAND ${CMAKE_COMMAND} 
    -DINPUT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/shaders 
    -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
    -P ${CMAKE_SOURCE_DIR}/scripts/compile_shaders.cmake
)