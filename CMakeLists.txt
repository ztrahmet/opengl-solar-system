cmake_minimum_required(VERSION 3.16)
project(solar-system VERSION 1.0)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find necessary libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# --- Define the executable target with the new name ---
add_executable(solar-system
  src/main.cpp
  src/glad.c
)

# --- NOW modify the target ---

# Add include directory
target_include_directories(solar-system PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the libraries
# Note the change from 'glm' to 'glm::glm'
target_link_libraries(solar-system PRIVATE
  OpenGL::GL
  glfw
  glm::glm 
)
