cmake_minimum_required(VERSION 3.14)
project(oly VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "Do not build in the source directory. Use 'cmake -B build'")
endif()

include(FetchContent)

FetchContent_Declare(
	yaml-cpp
	GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
	GIT_TAG        master
)
FetchContent_MakeAvailable(yaml-cpp)

set(SOURCES
	src/main.cpp
	src/log.cpp
	src/config.cpp
	src/contest.cpp
	src/utils.cpp
	src/cmds/add.cpp
	src/cmds/alias.cpp
	src/cmds/command.cpp
	src/cmds/default.cpp
	src/cmds/edit.cpp
	src/cmds/generate.cpp
	src/cmds/remove.cpp
	src/cmds/rename.cpp
	src/cmds/search.cpp
	src/cmds/show.cpp
)

add_executable(oly ${SOURCES})

target_compile_definitions(oly PRIVATE
	OLY_NAME="${PROJECT_NAME}"
	OLY_VERSION="${PROJECT_VERSION}"
	OLY_BUILD_TYPE="$<CONFIG>"
)

target_compile_features(oly PRIVATE cxx_std_26)

target_compile_options(oly PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(oly PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(oly PRIVATE yaml-cpp)

set_target_properties(oly PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
