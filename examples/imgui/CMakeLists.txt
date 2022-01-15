cmake_minimum_required(VERSION 3.21)

project(${PROJECT_NAME}_imgui LANGUAGES CXX)

find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_library(${PROJECT_NAME}_impl "impl/imgui_impl_win32.cpp" "impl/imgui_impl_glfw.cpp" "impl/imgui_impl_opengl3.cpp")
target_link_libraries(${PROJECT_NAME}_impl PUBLIC
    imgui::imgui
    glfw
    glad::glad
    coel::coel_cuiui
)

add_executable(${PROJECT_NAME}_main "main.cpp")
target_link_libraries(${PROJECT_NAME}_main PRIVATE ${PROJECT_NAME}_impl)
target_link_options(${PROJECT_NAME}_main PRIVATE -Wl,/subsystem:windows,/entry:mainCRTStartup)
