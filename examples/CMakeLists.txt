function (compile_shaders target)
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
        -DINPUT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/shaders 
        -DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_SOURCE_DIR}/scripts/compile_shaders.cmake
    )
endfunction()

function(copy_images)
    file(GLOB image_files ${CMAKE_CURRENT_SOURCE_DIR}/images/*)
    set(binary_dir ${CMAKE_CURRENT_BINARY_DIR})
    if (EXISTS ${binary_dir}/Debug)
        set(output_dir ${binary_dir}/Debug)
    elseif (EXISTS ${binary_dir}/debug)
        set(output_dir ${binary_dir}/debug)
    elseif (EXISTS ${binary_dir}/Release)
        set(output_dir ${binary_dir}/Release)
    elseif (EXISTS ${binary_dir}/release)
        set(output_dir ${binary_dir}/release)
    endif()
    foreach (image_file ${image_files})
        get_filename_component(${image_file} image NAME)
        file(COPY ${image_file} DESTINATION ${output_dir})
    endforeach()
endfunction()

add_subdirectory(black_window)
add_subdirectory(plain_rotation)
add_subdirectory(triangle_color)