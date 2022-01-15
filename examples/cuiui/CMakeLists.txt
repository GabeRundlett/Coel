cmake_minimum_required(VERSION 3.21)

project(${PROJECT_NAME}_cuiui LANGUAGES CXX)

find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(${PROJECT_NAME}_main "main.cpp")
target_link_libraries(${PROJECT_NAME}_main PRIVATE coel::coel_cuiui fmt::fmt)

add_executable(${PROJECT_NAME}_testing "testing.cpp")
target_link_libraries(${PROJECT_NAME}_testing PRIVATE coel::coel_cuiui fmt::fmt dwmapi)

add_executable(${PROJECT_NAME}_win32_d2d "win32_d2d.cpp")
target_link_libraries(${PROJECT_NAME}_win32_d2d PRIVATE fmt::fmt)

add_executable(${PROJECT_NAME}_win32_dwm "win32_dwm.cpp")
target_compile_features(${PROJECT_NAME}_win32_dwm PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME}_win32_dwm PRIVATE fmt::fmt glm::glm dwmapi)
# target_link_options(${PROJECT_NAME}_win32_dwm PRIVATE -Wl,/subsystem:windows,/entry:mainCRTStartup)
