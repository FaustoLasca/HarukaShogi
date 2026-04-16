cmake_minimum_required(VERSION 3.15)
project(HarukaShogi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mbmi2")

# Standalone executable
add_executable(HarukaShogi
    src/main.cpp
    src/position.cpp
    src/movegen.cpp
    src/misc.cpp
    src/perft.cpp
    src/evaluate.cpp
    src/search.cpp
    src/ttable.cpp
    src/opening_book.cpp
    src/bitboard.cpp
    src/movepicker.cpp
    src/thread.cpp
    src/usi.cpp
    src/engine.cpp
    src/nnue.cpp
)
target_include_directories(HarukaShogi PRIVATE include)


add_executable(test
    src/test.cpp
    src/position.cpp
    src/movegen.cpp
    src/misc.cpp
    src/perft.cpp
    src/evaluate.cpp
    src/search.cpp
    src/ttable.cpp
    src/opening_book.cpp
    src/bitboard.cpp
    src/movepicker.cpp
    src/thread.cpp
    src/usi.cpp
    src/engine.cpp
    src/nnue.cpp
)
target_include_directories(test PRIVATE include)

add_executable(book_generation
    src/book_generation/games_parser.cpp
    src/position.cpp
    src/movegen.cpp
    src/misc.cpp
    src/perft.cpp
    src/evaluate.cpp
    src/search.cpp
    src/ttable.cpp
    src/opening_book.cpp
    src/bitboard.cpp
    src/movepicker.cpp
    src/thread.cpp
    src/usi.cpp
    src/engine.cpp
    src/nnue.cpp
)
target_include_directories(book_generation PRIVATE include)