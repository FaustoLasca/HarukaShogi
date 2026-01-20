cmake_minimum_required(VERSION 3.15)
project(HarukaShogi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(searchengine
    src/bindings.cpp
    src/test.cpp
)

target_include_directories(searchengine PRIVATE include)

install(TARGETS searchengine
        LIBRARY DESTINATION .
)