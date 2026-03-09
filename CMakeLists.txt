cmake_minimum_required(VERSION 4.0) # I have no idea what to put so let's be safe
project(oly VERSION 0.1.1 LANGUAGES C CXX)

# compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(CMAKE_EXPORT_COMPILE_COMMANDS) # symlink in /build
	add_custom_target(link_compile_commands ALL
		COMMAND ${CMAKE_COMMAND} -E create_symlink
			${CMAKE_BINARY_DIR}/compile_commands.json
			${CMAKE_SOURCE_DIR}/build/compile_commands.json
	)
endif()

################
# dependencies #
################
include(FetchContent)

FetchContent_Declare(
	yaml-cpp
	GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
	GIT_TAG		master
)
FetchContent_MakeAvailable(yaml-cpp)

# treesitter
FetchContent_Declare(
	tree-sitter
	GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
	GIT_TAG        master
)
# Grammars
FetchContent_Declare(
	ts-latex
	GIT_REPOSITORY https://github.com/latex-lsp/tree-sitter-latex.git
	GIT_TAG        master
)
FetchContent_Declare(
	ts-typst
	GIT_REPOSITORY https://github.com/uben0/tree-sitter-typst
	GIT_TAG        master
)
FetchContent_MakeAvailable(tree-sitter ts-latex ts-typst)

message(STATUS "Generating Tree-sitter LaTeX parser...")
execute_process(
    COMMAND tree-sitter generate
    WORKING_DIRECTORY ${ts-latex_SOURCE_DIR}
    RESULT_VARIABLE TS_GEN_RESULT
)

if(NOT TS_GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to generate tree-sitter-latex parser. Is tree-sitter-cli installed?")
endif()

set(TS_NOISY_SOURCES
    ${ts-latex_SOURCE_DIR}/src/parser.c
    ${ts-latex_SOURCE_DIR}/src/scanner.c
    ${ts-typst_SOURCE_DIR}/src/parser.c
    ${ts-typst_SOURCE_DIR}/src/scanner.c
)

# disable warnings for just these files
if(MSVC)
	set_source_files_properties(${TS_NOISY_SOURCES} PROPERTIES COMPILE_FLAGS "/w")
else()
	set_source_files_properties(${TS_NOISY_SOURCES} PROPERTIES COMPILE_FLAGS "-w")
endif()

# get latest commit hash
execute_process(
	COMMAND git rev-parse --short HEAD
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_COMMIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

##############
# executable #
##############
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
	OLY_COMMIT_HASH="${GIT_COMMIT_HASH}"
)

target_compile_features(oly PRIVATE cxx_std_26)

target_compile_options(oly PRIVATE -Wall -Wextra -Wpedantic)

target_sources(oly PRIVATE
	${ts-latex_SOURCE_DIR}/src/parser.c
	${ts-latex_SOURCE_DIR}/src/scanner.c
	${ts-typst_SOURCE_DIR}/src/parser.c
	${ts-typst_SOURCE_DIR}/src/scanner.c
)

target_include_directories(oly PRIVATE
	${PROJECT_SOURCE_DIR}/include
	${tree-sitter_SOURCE_DIR}/lib/include
	${ts-latex_SOURCE_DIR}/src
	${ts-typst_SOURCE_DIR}/src
)

target_link_libraries(oly PRIVATE tree-sitter yaml-cpp)
