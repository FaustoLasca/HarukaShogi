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
    src/nnue/nnue.cpp
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
    src/nnue/nnue.cpp
    src/nnue/load.cpp
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
    src/nnue/nnue.cpp
)
target_include_directories(book_generation PRIVATE include)


add_executable(gensfen
    src/nnue/gensfen.cpp
    src/position.cpp
    src/movegen.cpp
    src/misc.cpp
    src/bitboard.cpp
    src/movepicker.cpp
    src/thread.cpp
    src/engine.cpp
    src/search.cpp
    src/ttable.cpp
    src/opening_book.cpp
    src/evaluate.cpp
    src/nnue/nnue.cpp
)
target_include_directories(gensfen PRIVATE include)


find_package(pybind11 REQUIRED)

pybind11_add_module(nnue_loader
    src/position.cpp
    src/movegen.cpp
    src/misc.cpp
    src/bitboard.cpp
    src/nnue/nnue.cpp
    src/nnue/load.cpp
    src/nnue/bindings.cpp
)
target_include_directories(nnue_loader PRIVATE include)


set(NNUE_WEIGHTS "${CMAKE_CURRENT_SOURCE_DIR}/bin/nnue/test_weights.bin")
set(BOOK_DATA "${CMAKE_CURRENT_SOURCE_DIR}/bin/book_data.bin")

set_source_files_properties(src/nnue/nnue.cpp
    PROPERTIES OBJECT_DEPENDS "${NNUE_WEIGHTS}"
)
set_source_files_properties(src/opening_book.cpp
    PROPERTIES OBJECT_DEPENDS "${BOOK_DATA}"
)

execute_process(
    COMMAND "${Python_EXECUTABLE}" -c
    "import site; print(site.getsitepackages()[0])"
    OUTPUT_VARIABLE PY_SITE_PACKAGES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
install(TARGETS nnue_loader LIBRARY DESTINATION "${PY_SITE_PACKAGES}")