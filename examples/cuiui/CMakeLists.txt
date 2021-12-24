cmake_minimum_required(VERSION 3.21)

project(${PROJECT_NAME}_cuiui LANGUAGES CXX)

find_package(fmt CONFIG REQUIRED)

add_executable(${PROJECT_NAME}_main "main.cpp")
target_link_libraries(${PROJECT_NAME}_main PRIVATE coel::coel_cuiui fmt::fmt)

add_executable(${PROJECT_NAME}_testing "testing.cpp")
target_link_libraries(${PROJECT_NAME}_testing PRIVATE coel::coel_cuiui fmt::fmt)
