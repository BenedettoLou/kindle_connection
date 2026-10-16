cmake_minimum_required(VERSION 3.15)
project(KindleConnection VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
set(SOURCES
    src/main.cpp
    src/utils.cpp
    src/config.cpp
    src/process_runner.cpp
)

# Header files
set(HEADERS
    include/utils.hpp
    include/config.hpp
    include/process_runner.hpp
)

add_executable(kindle_send ${SOURCES} ${HEADERS})

target_include_directories(kindle_send PRIVATE include)

# Compiler warnings and options
if(MSVC)
    target_compile_options(kindle_send PRIVATE /W4 /EHsc /utf-8)
else()
    target_compile_options(kindle_send PRIVATE -Wall -Wextra -pedantic)
endif()

# Installation target
install(TARGETS kindle_send DESTINATION bin)
