cmake_minimum_required(VERSION 3.12)

project(BuildAll CXX)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../" "${CMAKE_BINARY_DIR}/all/")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../examples" "${CMAKE_BINARY_DIR}/all/examples")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../test" "${CMAKE_BINARY_DIR}/all/test")
