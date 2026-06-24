cmake_minimum_required(VERSION 4.0)
project(crawler_detect_c C)

option(BUILD_TESTING "Build tests" ON)

set(CMAKE_C_STANDARD 99)

set(PCRE2_USE_STATIC_LIBS ON)

find_package(PCRE2 COMPONENTS 8BIT REQUIRED QUIET)

set(PROJECT_SOURCES
        src/crawler_detect.c
        src/fixtures/crawlers.c
        src/fixtures/exclusions.c
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PCRE2::8BIT)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests/unit)
endif ()

message(STATUS "Build tests: " ${BUILD_TESTING})
