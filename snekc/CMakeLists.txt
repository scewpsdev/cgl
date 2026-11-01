cmake_minimum_required(VERSION 3.15)
project(snekc VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    #add_compile_options(-Wall -Wextra)
    add_compile_options(-w)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -finline-functions)
    else()
        add_compile_options(-g -O0)
    endif()
endif()

set(SOURCE_FILES
    "src/Main.cpp"
    "src/tcc/tcc.c"
)

add_executable(snekc ${SOURCE_FILES})

# Headers unique to snekc
target_include_directories(snekc PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_compile_definitions(snekc PRIVATE _CONSOLE)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(snekc PRIVATE _DEBUG)
else()
    target_compile_definitions(snekc PRIVATE NDEBUG)
endif()

if (NOT WIN32)
    target_compile_definitions(snekc PRIVATE _GNU_SOURCE)
endif()

# Links the static library and pulls in its PUBLIC include paths automatically
target_link_libraries(snekc PRIVATE snek)