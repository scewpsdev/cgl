cmake_minimum_required(VERSION 3.15)
project(snek_workspace LANGUAGES C CXX)

# Pull in both subdirectories
add_subdirectory(snek)
add_subdirectory(snekc)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # -w disables ALL warnings completely on Linux (GCC/Clang)
    add_compile_options(-w) 
elseif(MSVC)
    # /W0 disables ALL warnings completely on Windows (MSVC)
    add_compile_options(/W0)
endif()

# Executables (e.g., .exe, ELF binaries)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Shared libraries / DLLs (e.g., .dll, .so)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Static libraries / Archives (e.g., .lib, .a)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")