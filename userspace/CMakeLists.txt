cmake_minimum_required(VERSION 3.16)
project(vsensor_monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# ------------------------------------------------------------------ #
#  Dependencies                                                       #
# ------------------------------------------------------------------ #

find_package(Threads REQUIRED)
find_package(Curses  REQUIRED)

# ------------------------------------------------------------------ #
#  Main executable                                                    #
# ------------------------------------------------------------------ #

add_executable(vsensor_monitor
    src/main.cpp
)

target_include_directories(vsensor_monitor PRIVATE
    include
    ${CURSES_INCLUDE_DIR}
)

target_link_libraries(vsensor_monitor PRIVATE
    Threads::Threads
    ${CURSES_LIBRARIES}
)

# ------------------------------------------------------------------ #
#  Unit tests (Google Test)                                           #
# ------------------------------------------------------------------ #

option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    enable_testing()
    find_package(GTest QUIET)

    if(GTest_FOUND)
        add_executable(vsensor_tests
            ../tests/test_circular_buffer.cpp
            ../tests/test_sensor_parser.cpp
            ../tests/test_stats_engine.cpp
            ../tests/test_alert_manager.cpp
        )
        target_include_directories(vsensor_tests PRIVATE
            include
        )
        target_link_libraries(vsensor_tests PRIVATE
            GTest::gtest
            GTest::gtest_main
            Threads::Threads
        )
        include(GoogleTest)
        gtest_discover_tests(vsensor_tests)
    else()
        message(WARNING "GTest not found – unit tests will not be built.")
    endif()
endif()

# ------------------------------------------------------------------ #
#  Doxygen documentation                                              #
# ------------------------------------------------------------------ #

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../docs/doxygen)
    doxygen_add_docs(docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMENT "Generating API documentation with Doxygen"
    )
endif()
