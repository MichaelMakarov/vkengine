add_library(
    engine_geometry STATIC 
    vertex.cpp
)
target_include_directories(
    engine_geometry PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(
    engine_geometry PUBLIC 
    glm::glm
    engine_graphics
)