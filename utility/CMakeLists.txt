add_library(
    utility STATIC
    src/error.cpp 
)
target_include_directories(
    utility PUBLIC include
)