cmake_minimum_required(VERSION 3.12)
set(Project result_test)
project(${Project} CXX)

# ------------------------------------- Handle sanitisers settings -------------------------------------

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(IS_TOPLEVEL_PROJECT TRUE)
else()
    set(IS_TOPLEVEL_PROJECT FALSE)
endif()
if(IS_TOPLEVEL_PROJECT)
   if(sanitiser_mode STREQUAL none OR NOT sanitiser_mode)
   else()
      add_compile_options(-fno-omit-frame-pointer -fno-optimize-sibling-calls
                          # -fsanitize-blacklist=${BLACKLIST_PATH}
                          )
      if(sanitiser_mode STREQUAL asan)
         # https://clang.llvm.org/docs/AddressSanitizer.html
         add_link_options(-fsanitize=address)
         add_compile_options(-fsanitize=address)
      elseif(sanitiser_mode STREQUAL ubsan)
         # https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
         # https://github.com/llvm/llvm-project/issues/80
         add_link_options(-fsanitize=undefined -fno-sanitize=vptr)
         add_compile_options(-fsanitize=undefined -fno-sanitize=vptr)
      elseif(sanitiser_mode STREQUAL msan)
         # https://clang.llvm.org/docs/MemorySanitizer.html
         add_link_options(-fsanitize=memory)
         add_compile_options(-fsanitize=memory
                             -fsanitize-memory-track-origins
                             -fsanitize-memory-use-after-dtor
         )
      elseif(sanitiser_mode STREQUAL tsan)
         # https://clang.llvm.org/docs/MemorySanitizer.html
         add_link_options(-fsanitize=thread)
         add_compile_options(-fsanitize=thread)
      elseif(sanitiser_mode STREQUAL srccov)
         # Do nothing right now, since we do not want to compile 3rd party contrib tools
         # with code coverage. Add required flags below.
      else()
         message(FATAL_ERROR "sanitiser_mode specified (and equals to \"${sanitiser_mode}\") but this mode is unknown."
                              " Available modes are: none, asan, ubsan, msan")
      endif()
   endif()
endif()


# ------------------------------------- -------------------------------------
if (NOT TARGET result)
   include(FetchContent)
   FetchContent_Declare(result SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
   FetchContent_MakeAvailable(result)
endif()

# ------------------------------------- -------------------------------------
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


# ------------------------------------- -------------------------------------

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
