cmake_minimum_required(VERSION 3.15)
project(QuantisedEval LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type: Debug Release" FORCE)
endif ()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Compiler flags
add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -pedantic
        -pthread
)

# Arch-specific optimization
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-march=native" HAS_MARCH_NATIVE)
if (HAS_MARCH_NATIVE)
    add_compile_options(-march=native)
endif ()

# Release flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -funroll-loops")

# Debug/Test flags (sanitizers)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -fsanitize=address,undefined,leak -fno-omit-frame-pointer")

# Eval file definition (source directory)
set(EVALFILE "quantised.bin" CACHE STRING "Path to evaluation file in source directory")
add_definitions(-DEVALFILE=\"${EVALFILE}\")

# Source files
set(SOURCES
        schoenemann.cpp
        search.cpp
        timeman.cpp
        helper.cpp
        tt.cpp
        moveorder.cpp
        see.cpp
        tune.cpp
        datagen.cpp
        history.cpp
        NNUE/nnue.cpp
)

# Add executable
add_executable(null ${SOURCES})

# Link threading library
find_package(Threads REQUIRED)
target_link_libraries(null PRIVATE Threads::Threads)

# Copy the evaluation file from parent source directory to build output
add_custom_command(TARGET null POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/${EVALFILE}
        $<TARGET_FILE_DIR:null>    # copy next to the executable
)
