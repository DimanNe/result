cmake_minimum_required(VERSION 3.12)
set(Project result_test)
project(${Project} CXX)

if (NOT TARGET result)
   include(FetchContent)
   FetchContent_Declare(result SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
   FetchContent_MakeAvailable(result)
endif()

# GTest needs to be compiled with "our" options to avoid undefined references to libc++ symbols
add_compile_options(-stdlib=libc++)
add_link_options(-stdlib=libc++)
if (NOT TARGET gtest)
   include(FetchContent)
   FetchContent_Declare(
      gtest
      GIT_REPOSITORY      https://github.com/google/googletest.git
      GIT_TAG             870a6b5563e065b4b9fc3b633c4015ee17d2cbaa
      UPDATE_DISCONNECTED YES
   )
   FetchContent_MakeAvailable(gtest)
endif()


add_executable(${Project}
   coresult.cpp
   result.cpp
)
target_link_libraries(${Project} PRIVATE
   result
   gmock
   gtest
   gtest_main
)
enable_testing()
add_test(${Project} ${Project})
