cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(vkengine_project)

find_package(Vulkan REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(GLM REQUIRED)

add_subdirectory(engine)
add_subdirectory(examples)
