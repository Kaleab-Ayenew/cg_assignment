cmake_minimum_required(VERSION 3.14)
project(TicTacToe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Fetch Dependencies using FetchContent ---
include(FetchContent)

# Fetch GLM (math library)
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Fetch STB (for font loading)
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)
FetchContent_MakeAvailable(stb)


# --- Find System Dependencies ---
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

# --- Project Sources ---
add_executable(tictactoe
    src/main.cpp
    src/glad.c
)

# --- Include Directories ---
target_include_directories(tictactoe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${glm_SOURCE_DIR}
    ${stb_SOURCE_DIR}
)

# --- Linking ---
target_link_libraries(tictactoe PRIVATE
    glfw
    ${OPENGL_LIBRARIES}
)

# --- Platform Specifics ---
if (APPLE)
    target_link_libraries(tictactoe PRIVATE "-framework Cocoa -framework IOKit -framework CoreVideo")
endif()

# --- Output ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
