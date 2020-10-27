# Copyright(C) 2020 Henry Bullingham
# This file is subject to the license terms in the LICENSE file
# found in the top - level directory of this distribution.


if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp)
    file(DOWNLOAD
         https://raw.githubusercontent.com/catchorg/Catch2/master/single_include/catch2/catch.hpp
         ${CMAKE_CURRENT_BINARY_DIR}/catch.hpp
         STATUS status
         LOG log)

    list(GET status 0 status_code)
    list(GET status 1 status_string)

    if(NOT status_code EQUAL 0)
        message(FATAL_ERROR "error downloading catch: ${status_string}"
                "${log}")
    endif()
endif()

set(source_files 
    test_main.cpp
	cppcosl_test.cpp
    )
                 
add_executable(cppcosl_test ${source_files})
target_link_libraries(cppcosl_test PUBLIC cppdj)
target_include_directories(cppcosl_test PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET cppcosl_test PROPERTY CXX_STANDARD 11)
add_test(NAME test COMMAND cppcosl_test)