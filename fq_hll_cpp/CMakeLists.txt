cmake_minimum_required(VERSION 3.10)
project(fq_hll VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)

include(GNUInstallDirs)

install(TARGETS fq_hll
    EXPORT fq_hllTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(PROGRAMS scripts/fq_hll_g++ scripts/fq_hll_clang++
    DESTINATION ${CMAKE_INSTALL_BINDIR})

# Export CMake config files
install(EXPORT fq_hllTargets
    FILE fq_hllTargets.cmake
    NAMESPACE fq_hll::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fq_hll)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/fq_hllConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/fq_hllConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/fq_hllConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fq_hll)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/fq_hllConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/fq_hllConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fq_hll)
