cmake_minimum_required(VERSION 3.15)
project(HarukaShogi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mbmi2")

# Pybind11 configuration
# set(PYBIND11_FINDPYTHON ON)
# find_package(pybind11 CONFIG REQUIRED)

# pybind11_add_module(haruka
#     src/bindings.cpp
#     src/position.cpp
#     src/movegen.cpp
#     src/perft.cpp
#     src/evaluate.cpp
#     src/search.cpp
#     src/misc.cpp
#     src/ttable.cpp
#     src/opening_book.cpp
#     src/book_data.cpp
#     src/bitboard.cpp
#     src/movepicker.cpp
#     src/thread.cpp
#     src/usi.cpp
# )

# target_include_directories(haruka PRIVATE include)

# Standalone executable
add_executable(test_types
    src/main.cpp
    src/position.cpp
    src/movegen.cpp
    src/misc.cpp
    src/perft.cpp
    src/evaluate.cpp
    src/search.cpp
    src/ttable.cpp
    src/opening_book.cpp
    src/book_data.cpp
    src/bitboard.cpp
    src/movepicker.cpp
    src/thread.cpp
    src/usi.cpp
    src/engine.cpp
)
target_include_directories(test_types PRIVATE include)

# # Install targets (install searchengine library in the python environment)
# install(TARGETS haruka
#         LIBRARY DESTINATION .
# )