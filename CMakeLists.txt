cmake_minimum_required(VERSION 3.14)
project(oly LANGUAGES CXX)

include(FetchContent)

# generate compile_commands.json for LSP tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fetch yaml-cpp from GitHub
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(yaml-cpp)

# Gather all source files
file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Create executable
add_executable(oly ${SOURCES})

# Require C++23 for oly target
target_compile_features(oly PRIVATE cxx_std_23)

# Add include directory
target_include_directories(oly PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Link yaml-cpp
target_link_libraries(oly PRIVATE yaml-cpp)
