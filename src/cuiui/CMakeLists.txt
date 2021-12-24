cmake_minimum_required(VERSION 3.21)

project(coel_cuiui LANGUAGES CXX)
find_package(glad CONFIG REQUIRED)

set(PLATFORM_SRCS)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND PLATFORM_SRCS "src/platform/x11/window.cpp")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND PLATFORM_SRCS "src/platform/win32/window.cpp")
endif()

add_library(${PROJECT_NAME} "src/ui_context.cpp" ${PLATFORM_SRCS})
add_library(coel::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad opengl32)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

install(TARGETS ${PROJECT_NAME} EXPORT coel-targets
    COMPONENT core
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY "include/" TYPE INCLUDE PATTERN "*.hpp")
