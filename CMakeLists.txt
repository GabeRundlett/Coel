cmake_minimum_required(VERSION 3.21)
project(coel VERSION 0.1.0)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_library(coel "src/coel.cpp")
add_library(coel::coel ALIAS coel)
target_compile_features(coel PUBLIC cxx_std_20)
if(BUILD_SHARED_LIBS AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(coel
        PRIVATE "COEL_EXPORT=__declspec(dllexport)"
        INTERFACE "COEL_EXPORT=__declspec(dllimport)")
else()
    target_compile_definitions(coel
        PUBLIC "COEL_EXPORT=__declspec(dllexport)")
endif()
target_include_directories(coel PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

file(WRITE ${CMAKE_BINARY_DIR}/config.cmake.in [=[
@PACKAGE_INIT@
include(${CMAKE_CURRENT_LIST_DIR}/coel-targets.cmake)
check_required_components(coel)
]=])
configure_package_config_file(${CMAKE_BINARY_DIR}/config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/coel-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/coel
    NO_SET_AND_CHECK_MACRO)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/coel-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/coel-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/coel-config-version.cmake
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}/coel)

install(TARGETS coel EXPORT coel-targets)
install(EXPORT coel-targets DESTINATION ${CMAKE_INSTALL_DATADIR}/coel NAMESPACE coel::)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/ TYPE INCLUDE)
