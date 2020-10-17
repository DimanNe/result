cmake_minimum_required(VERSION 3.12)

project(BuildAll CXX)

foreach(name IN ITEMS test examples)
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../${name}" "${CMAKE_BINARY_DIR}/all/${name}")
endforeach()
