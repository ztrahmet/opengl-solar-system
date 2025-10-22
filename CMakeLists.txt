cmake_minimum_required(VERSION 3.16)
project(solar-system VERSION 1.0)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find necessary libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Define the executable target
add_executable(solar-system
  src/main.cpp
  src/glad.c
  src/shader.cpp
  src/camera.cpp
  src/planet.cpp
  src/ini.c       
  src/config.cpp  
  src/stb_image.cpp # <-- Ensure this line is present
)

# Add include directory
target_include_directories(solar-system PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the libraries
target_link_libraries(solar-system PRIVATE
  OpenGL::GL
  glfw
  glm::glm
  Threads::Threads
)

# --- Asset Copying ---
add_custom_command(TARGET solar-system POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
  COMMENT "Copying shaders to build directory"
)

add_custom_command(TARGET solar-system POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/textures ${CMAKE_CURRENT_BINARY_DIR}/textures
  COMMENT "Copying textures to build directory"
)

add_custom_command(TARGET solar-system POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/config.ini ${CMAKE_CURRENT_BINARY_DIR}/config.ini
  COMMENT "Copying config.ini to build directory"
)
