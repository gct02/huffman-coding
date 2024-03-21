# Minimum required CMake version
cmake_minimum_required(VERSION 3.18)

project("huffman")

# Define a C++ standard
set(CMAKE_CXX_STANDARD 20)

# Set paths to source files
set(SOURCES
    "src/scoped-handler.hpp"
    "src/huffman-tree.hpp"
    "src/huffman-encoder.hpp"
    "src/huffman.hpp"
    "src/path-manager.h"
    "src/path-manager.cpp"
    "src/messages.h"
    "src/messages.cpp"
    "src/main.cpp"
)

# Add source files to this project's executable
add_executable(huffman ${SOURCES})

# Output directory for the executable
set_target_properties(huffman PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Ensure the required C++ standard is used to build this project
set_property(TARGET huffman PROPERTY CXX_STANDARD 20)
