# Copyright (c) 2025 Eray Ozturk <erayozturk1@gmail.com>
# url github.com/diffstorm
cmake_minimum_required(VERSION 3.12)
project(ini_parser LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library configuration
add_library(ini_parser STATIC
    ini_parser.h
    ini_parser.c
)

set_target_properties(ini_parser PROPERTIES
    C_VISIBILITY_PRESET hidden
    INTERPROCEDURAL_OPTIMIZATION FALSE
)

target_include_directories(ini_parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# C demo
add_executable(demo
    demo.c
)

# C++17 demo
add_executable(demo_cpp
    demo.cpp
)

# C++17 demo
add_executable(demo_stream
    demo_stream.cpp
)

# Link demo to library
target_link_libraries(demo PRIVATE ini_parser)
target_link_libraries(demo_cpp PRIVATE ini_parser)
target_link_libraries(demo_stream PRIVATE ini_parser)

# Google Test configuration
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Test executable
add_executable(ini_parser_tests
    ini_parser_tests.cpp
)

target_link_libraries(ini_parser_tests
    PRIVATE
    ini_parser
    GTest::GTest
    GTest::Main
    Threads::Threads
)

enable_testing()
add_test(NAME ini_parser_tests COMMAND ini_parser_tests)

# Installation configuration
install(TARGETS ini_parser
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION include
)

install(FILES ini_parser.h
    DESTINATION include
)
