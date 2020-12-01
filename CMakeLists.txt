cmake_minimum_required(VERSION 3.16.3)

set(PROJECT snake)

# file(TO_NATIVE_PATH ${CMAKE_SOURCE_DIR}/src SRC)
file(GLOB SRC ${CMAKE_SOURCE_DIR}/src/*.cxx)

project(${PROJECT})
add_executable(${PROJECT} ${SRC})

target_compile_options(${PROJECT} PRIVATE -Wall -Werror -Wextra -pedantic -fmax-errors=3)

find_package(Curses REQUIRED)
target_link_libraries(${PROJECT} ${CURSES_LIBRARIES})
target_include_directories(${PROJECT} PRIVATE ${CURSES_INCLUDE_DIR})

set_property(TARGET ${PROJECT} PROPERTY CXX_STANDARD 20)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "Build type: Release")
endif()
