cmake_minimum_required(VERSION 3.12)

project(result_examples CXX)

include(FetchContent)
FetchContent_Declare(result SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
FetchContent_MakeAvailable(result)

add_executable(result_examples source/main.cpp)

target_compile_features(result_examples PRIVATE cxx_std_20)

target_compile_options(result_examples
        PRIVATE
        "$<$<CXX_COMPILER_ID:MSVC>:/await /permissive->")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(result_examples PRIVATE -stdlib=libc++)
endif()

target_include_directories(result_examples PRIVATE "${PROJECT_SOURCE_DIR}/include>")

target_link_libraries(result_examples PRIVATE DimanNe::result)
