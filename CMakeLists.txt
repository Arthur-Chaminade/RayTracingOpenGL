cmake_minimum_required(VERSION 3.10)
project(OpenGLApp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set path to libraries
set(GLFW_DIR "${CMAKE_SOURCE_DIR}/libraries/GLFW/")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/libraries/imgui/")
set(ASSIMP_DIR "${CMAKE_SOURCE_DIR}/libraries/Assimp/")
set(ZLIB_DIR "${CMAKE_SOURCE_DIR}/libraries/zlib/")
# Include directories
include_directories(${GLFW_DIR}/include)
include_directories(${IMGUI_DIR}/include)
include_directories(${ASSIMP_DIR}/include)
include_directories(${ZLIB_DIR}/include)


find_package(OpenGL REQUIRED)

file(GLOB SOURCES "src/*.cpp" "src/*.c" "${IMGUI_DIR}/*.cpp")
add_executable(OpenGLApp ${SOURCES})
target_include_directories(OpenGLApp PRIVATE include)
target_link_libraries(OpenGLApp OpenGL::GL ${GLFW_DIR}/lib/libglfw3.a ${ASSIMP_DIR}/lib/libassimp.a ${ZLIB_DIR}/lib/libzlibstatic.a)
