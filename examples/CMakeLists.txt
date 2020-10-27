# Copyright(C) 2020 Henry Bullingham
# This file is subject to the license terms in the LICENSE file
# found in the top - level directory of this distribution.


add_executable(cppcosl_example_basic_coroutines basic_coroutines.cpp)            
target_link_libraries(cppcosl_example_basic_coroutines PUBLIC cppcosl)
target_include_directories(cppcosl_example_basic_coroutines PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET cppcosl_example_basic_coroutines PROPERTY CXX_STANDARD 11)

add_executable(cppcosl_example_nested_coroutines nested_coroutines.cpp)            
target_link_libraries(cppcosl_example_nested_coroutines PUBLIC cppcosl)
target_include_directories(cppcosl_example_nested_coroutines PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET cppcosl_example_nested_coroutines PROPERTY CXX_STANDARD 11)

add_executable(cppcosl_example_joining_coroutines joining_coroutines.cpp)            
target_link_libraries(cppcosl_example_joining_coroutines PUBLIC cppcosl)
target_include_directories(cppcosl_example_joining_coroutines PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET cppcosl_example_joining_coroutines PROPERTY CXX_STANDARD 11)