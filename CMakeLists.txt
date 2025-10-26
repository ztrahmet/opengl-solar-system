cmake_minimum_required(VERSION 3.16)
project(solar-system VERSION 1.0)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Fetch Dear ImGui using FetchContent ---
include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        docking # Or a specific commit hash/tag like v1.89.9
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)
# --- End Fetch Dear ImGui ---

# Find necessary libraries
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(X11 REQUIRED)

# Define the executable target
add_executable(solar-system
  src/main.cpp
  src/glad.c
  src/shader.cpp
  src/camera.cpp
  src/planet.cpp
  src/ini.c
  src/config.cpp
  src/stb_image.cpp
  src/scenario.cpp
  # --- Add ImGui core source files directly ---
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  # --- Add ImGui backend files ---
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Add include directories
target_include_directories(solar-system PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${imgui_SOURCE_DIR}              # Include dir for imgui.h
  ${imgui_SOURCE_DIR}/backends     # Include dir for backends
  ${X11_INCLUDE_DIR}
)

# Link the libraries
target_link_libraries(solar-system PRIVATE
  OpenGL::GL
  glfw
  glm::glm
  Threads::Threads
  ${X11_LIBRARIES}
)

# --- Asset Copying --- (Same as before)
add_custom_command(TARGET solar-system POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
  COMMENT "Copying shaders to build directory"
)
add_custom_command(TARGET solar-system POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/textures ${CMAKE_CURRENT_BINARY_DIR}/textures
  COMMENT "Copying textures (including skybox) to build directory"
)
add_custom_command(TARGET solar-system POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/config.ini ${CMAKE_CURRENT_BINARY_DIR}/config.ini
  COMMENT "Copying config.ini to build directory"
)
