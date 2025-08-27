cmake_minimum_required(VERSION 3.14)
project(oly LANGUAGES CXX)

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

# FetchContent_Declare(
# 	json
# 	GIT_REPOSITORY https://github.com/nlohmann/json.git
# 	GIT_TAG        v3.11.2
# )
# FetchContent_MakeAvailable(json)

# FetchContent_Declare(
# 	cpr
# 	GIT_REPOSITORY https://github.com/libcpr/cpr.git
# 	GIT_TAG 1.10.5
# )
# FetchContent_MakeAvailable(cpr)

# FetchContent_Declare(
# 	gumbo
# 	GIT_REPOSITORY https://github.com/google/gumbo-parser.git
# 	GIT_TAG        main
# )
# FetchContent_MakeAvailable(gumbo)

set(SOURCES
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

add_executable(oly ${SOURCES})

target_compile_features(oly PRIVATE cxx_std_23)
target_compile_options(oly PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(oly PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(oly PRIVATE yaml-cpp #[[ nlohmann_json::nlohmann_json cpr::cpr gumbo ]])

set_target_properties(oly PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
