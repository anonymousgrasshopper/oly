cmake_minimum_required(VERSION 3.14)
project(oly VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(CMAKE_EXPORT_COMPILE_COMMANDS)
	add_custom_target(link_compile_commands ALL
		COMMAND ${CMAKE_COMMAND} -E create_symlink
			${CMAKE_BINARY_DIR}/compile_commands.json
			${CMAKE_SOURCE_DIR}/build/compile_commands.json
	)
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
	src/cmds/get_cmd.cpp
	src/cmds/remove.cpp
	src/cmds/rename.cpp
	src/cmds/search.cpp
	src/cmds/show.cpp
)

add_executable(oly ${SOURCES})

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

target_compile_definitions(oly PRIVATE
	OLY_NAME="${PROJECT_NAME}"
	OLY_VERSION="${PROJECT_VERSION}"
	OLY_BUILD_TYPE="$<CONFIG>"
	OLY_COMMIT_HASH="${GIT_COMMIT_HASH}"
)

target_compile_features(oly PRIVATE cxx_std_26)

target_compile_options(oly PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(oly PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(oly PRIVATE yaml-cpp)
