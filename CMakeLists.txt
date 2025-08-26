cmake_minimum_required(VERSION 3.14)
project(oly LANGUAGES CXX)

include(FetchContent)

# generate compile_commands.json for LSP tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# build the code in build/
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

# Fetch yaml-cpp from GitHub
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(yaml-cpp)

# Create executable
add_executable(oly
	src/main.cpp
	src/log.cpp
	src/config.cpp
	src/utils.cpp
	src/cmds/add.cpp
	src/cmds/command.cpp
	src/cmds/default.cpp
	src/cmds/edit.cpp
	src/cmds/generate.cpp
	src/cmds/search.cpp
	src/cmds/show.cpp
)

# Require C++23 for oly target
target_compile_features(oly PRIVATE cxx_std_23)

# Add include directory
target_include_directories(oly PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Link yaml-cpp
target_link_libraries(oly PRIVATE yaml-cpp)
