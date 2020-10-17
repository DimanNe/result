cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(diresult_root LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(IS_TOPLEVEL_PROJECT TRUE)
else()
    set(IS_TOPLEVEL_PROJECT FALSE)
endif()

##############################################################################################################
# Set options/settings for our projects if we are top-level project

if(IS_TOPLEVEL_PROJECT)
   set(CMAKE_CXX_STANDARD 20)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   set(CMAKE_CXX_EXTENSIONS OFF)
   add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()


##############################################################################################################
# Tell CMake what to build

add_subdirectory(src)
add_subdirectory(examples)

if(NOT IS_TOPLEVEL_PROJECT) # Tests require (at least) gtest/gmock, which we do not provide being a top-level project
   enable_testing()
   add_subdirectory(ut)
endif()
