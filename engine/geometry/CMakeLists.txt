add_library(
    engine_geometry INTERFACE
)
target_include_directories(
    engine_geometry INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    engine_geometry INTERFACE 
    glm::glm
)