cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(diresult_root LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(IS_TOPLEVEL_PROJECT TRUE)
else()
    set(IS_TOPLEVEL_PROJECT FALSE)
endif()


##############################################################################################################
# Setup sanitisers' flags if we are top-level project
##############################################################################################################

if(IS_TOPLEVEL_PROJECT)
   if(sanitiser_mode STREQUAL none OR NOT sanitiser_mode)
   else()
      set(BLACKLIST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/sanitiser_blacklist.txt")
      #string(APPEND BLACKLIST_PATH)
      add_compile_options(-fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-blacklist=${BLACKLIST_PATH})
      if(sanitiser_mode STREQUAL asan) # https://clang.llvm.org/docs/AddressSanitizer.html
         add_link_options(-fsanitize=address)
         add_compile_options(-fsanitize=address)
      elseif(sanitiser_mode STREQUAL ubsan) # https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html https://github.com/llvm/llvm-project/issues/80
         add_link_options(-fsanitize=undefined -fno-sanitize=vptr)
         add_compile_options(-fsanitize=undefined -fno-sanitize=vptr)
      elseif(sanitiser_mode STREQUAL msan) # https://clang.llvm.org/docs/MemorySanitizer.html
         add_link_options(-fsanitize=memory)
         add_compile_options(-fsanitize=memory
                             -fsanitize-memory-track-origins
                             -fsanitize-memory-use-after-dtor
      )
      elseif(sanitiser_mode STREQUAL tsan) # https://clang.llvm.org/docs/MemorySanitizer.html
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



##############################################################################################################
# Set options/settings for our projects if we are top-level project
##############################################################################################################
if(IS_TOPLEVEL_PROJECT)
   set(CMAKE_CXX_STANDARD 20)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   set(CMAKE_CXX_EXTENSIONS OFF)
   add_compile_options(-Wall -Wextra -pedantic -Werror)
   # include_directories(. src)
   if(sanitiser_mode STREQUAL srccov)
      # https://clang.llvm.org/docs/MemorySanitizer.html
      add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
      add_link_options(-fprofile-instr-generate -fcoverage-mapping)
   endif()
endif()


##############################################################################################################
# Tell CMake what our projects are
##############################################################################################################
add_subdirectory(src)
add_subdirectory(examples)
if(NOT IS_TOPLEVEL_PROJECT)
   enable_testing()
   add_subdirectory(ut)
endif()
