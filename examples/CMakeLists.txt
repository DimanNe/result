cmake_minimum_required(VERSION 3.12)

project(result_examples CXX)

include(FetchContent)
FetchContent_Declare(result SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
FetchContent_GetProperties(result)
if(NOT result_POPULATED)
  FetchContent_Populate(result)
  FetchContent_MakeAvailable(result)
endif()

add_executable(result_examples source/main.cpp)
target_link_libraries(result_examples PRIVATE DimanNe::result)
