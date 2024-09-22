function (compile_shaders target)
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
        -DINPUT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/shaders 
        -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_SOURCE_DIR}/scripts/compile_shaders.cmake
    )
endfunction()

add_subdirectory(colored_window)
add_subdirectory(hardcoded_triangle)
#add_subdirectory(loaded_triangle)