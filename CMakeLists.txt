cmake_minimum_required(VERSION 3.16)
project(solar-system VERSION 1.0)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find necessary libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Define the executable target and include ALL source files
add_executable(solar-system
  src/main.cpp
  src/glad.c
  src/shader.cpp
  src/camera.cpp
)

# Add include directory so the compiler can find shader.h and camera.h
target_include_directories(solar-system PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the libraries
target_link_libraries(solar-system PRIVATE
  OpenGL::GL
  glfw
  glm::glm 
)

# --- NEW: Copy assets to the build directory ---
# This command runs after the build is complete, ensuring the shaders
# are next to the executable.
add_custom_command(TARGET solar-system POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders" "${CMAKE_BINARY_DIR}/shaders"
    COMMENT "Copying shaders to build directory"
)
