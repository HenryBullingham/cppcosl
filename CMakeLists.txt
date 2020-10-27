# Copyright(C) 2020 Henry Bullingham
# This file is subject to the license terms in the LICENSE file
# found in the top - level directory of this distribution.


cmake_minimum_required(VERSION 3.1)

project(cppcosl)

set(header_files 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/cppcosl/cppcosl.h)

add_library(cppcosl INTERFACE)
target_sources(cppcosl INTERFACE "$<BUILD_INTERFACE:${header_files}>")
target_include_directories(cppcosl INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>")
target_include_directories(cppcosl SYSTEM INTERFACE "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/>")


# Setup package config
include( CMakePackageConfigHelpers )
set(CONFIG_PACKAGE_INSTALL_DIR lib/cmake/cppcosl)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cppcosl-config-version.cmake
    VERSION 0.1.0
    COMPATIBILITY SameMajorVersion)

# Install target
install(DIRECTORY include/cppcosl DESTINATION include)

enable_testing()
install(TARGETS cppcosl
    EXPORT cppcosl-targets
    DESTINATION lib)

install( EXPORT cppcosl-targets
    DESTINATION
    ${CONFIG_PACKAGE_INSTALL_DIR})

install( FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/cppcosl-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cppcosl-config-version.cmake
    DESTINATION
    ${CONFIG_PACKAGE_INSTALL_DIR} )
    
option(CPPCOSL_BUILD_TEST "build tests" ON)
if(${CPPCOSL_BUILD_TEST})
    add_subdirectory(test/)
endif()

option(CPPCOSL_BUILD_EXAMPLES "build examples" ON)
if(${CPPCOSL_BUILD_EXAMPLES})
    add_subdirectory(examples/)
endif()
