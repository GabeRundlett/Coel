cmake_minimum_required(VERSION 3.21)
project(coel)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_subdirectory("src/cuiui")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/coel-config.cmake"
    INSTALL_DESTINATION "share/coel"
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/coel-config.cmake"
    DESTINATION "share/coel"
)
install(EXPORT coel-targets
    FILE coel-targets.cmake
    NAMESPACE coel::
    DESTINATION "share/coel"
)

add_subdirectory(examples)
