cmake_minimum_required(VERSION 3.16)
project(obewrong LANGUAGES C CXX)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif ()
# Find LLVM
find_package(LLVM REQUIRED CONFIG)

# Print LLVM information
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVM include dir: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Using LLVM libraries: ${LLVM_LIBRARIES}")

# Include LLVM directories
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Set up source files
include_directories("include")
file(GLOB_RECURSE SOURCES "src/*.cc")
file(GLOB_RECURSE HEADERS "include/*.h")

add_library(obewrong_lib STATIC ${SOURCES} ${HEADERS})

# Link against LLVM components
llvm_map_components_to_libnames(
        LLVM_LIBS
        core support
        irreader
        bitreader bitwriter
        codegen
        target x86asmparser x86codegen
        linker
        targetparser
        )
target_link_libraries(obewrong_lib PRIVATE ${LLVM_LIBS})

add_executable(obewrong src/main.cc)
target_link_libraries(obewrong PRIVATE obewrong_lib)

# Enable warnings
target_compile_options(obewrong PRIVATE
        -Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations -Wno-unused-parameter
)

# Enable optimizations in Release mode
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(obewrong PRIVATE -O2)
endif()
