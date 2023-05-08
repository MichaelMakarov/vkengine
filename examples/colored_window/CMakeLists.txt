add_executable(
    colored_window 
    src/main.cpp
)
target_link_libraries(
    colored_window PRIVATE engine
)