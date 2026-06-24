cmake_minimum_required(VERSION 4.0)
project(crawler_detect_c_tests C)

set(CMAKE_C_STANDARD 99)

set(TEST_SOURCES
        utest.h
        main.c
        test_crawler_detect.c
)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} ${CMAKE_PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE CRAWLER_DETECT_TESTS_DATA="${CMAKE_SOURCE_DIR}/tests/unit/data")
