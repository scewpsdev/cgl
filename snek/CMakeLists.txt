cmake_minimum_required(VERSION 3.15)
project(snek VERSION 1.0 LANGUAGES C CXX)

# Force C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Determine the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler optimization settings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    #add_compile_options(-Wall -Wextra)
    add_compile_options(-w)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
    else()
        add_compile_options(-g -O0)
    endif()
endif()

# Explicitly list all source files from your vcxproj
# (CMake handles headers automatically, but you can add them here if you want them visible in IDEs)
set(SNEK_SOURCE_FILES
    "src/codegen/CodeBuffer.cpp"
    "src/codegen/Codegen.cpp"
    "src/File.cpp"
    "src/parser/AST.cpp"
    "src/parser/Diagnostics.cpp"
    "src/parser/Lexer.cpp"
    "src/parser/Parser.cpp"
    "src/Platform.cpp"
    "src/typechecker/TypeChecker.cpp"
    "src/typechecker/TypeSystem.cpp"
    "src/utils/Arena.cpp"
    "src/utils/Hash.cpp"
    "src/utils/ScratchBuffer.cpp"
    "src/utils/StringBuffer.cpp"
    "src/utils/StringView.cpp"
    "src/utils/TypeSet.cpp"
)

# Declare snek as a static library
add_library(snek STATIC ${SNEK_SOURCE_FILES})

# Expose include directories to any target that links against 'snek'
target_include_directories(snek PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Set preprocessor macros
target_compile_definitions(snek PRIVATE _CONSOLE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(snek PRIVATE _DEBUG)
else()
    target_compile_definitions(snek PRIVATE NDEBUG)
endif()

if (NOT WIN32)
    target_compile_definitions(snek PRIVATE _CRT_SECURE_NO_WARNINGS _GNU_SOURCE)
endif()