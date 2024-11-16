include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(geometry)
add_subdirectory(graphics)
add_subdirectory(image)
add_subdirectory(utility)

add_library(engine INTERFACE)
target_link_libraries(engine INTERFACE engine_graphics engine_utility engine_geometry)
target_include_directories(engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})