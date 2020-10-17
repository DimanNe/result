cmake_minimum_required(VERSION 3.12)

project(dimanne_result VERSION 0.0.1 LANGUAGES CXX)

# ---- Warning guard ----

# Protect dependents from this project's warnings if the guard isn't disabled
set(result_warning_guard "SYSTEM")
if(result_INCLUDE_WITHOUT_SYSTEM)
  set(result_warning_guard "")
endif()

# ---- Define library ----

add_library(result INTERFACE)
add_library(DimanNe::result ALIAS result)

target_compile_features(result INTERFACE cxx_std_20)

target_compile_options(result
        INTERFACE
        "$<$<CXX_COMPILER_ID:MSVC>:/await /permissive->")

target_include_directories(result
        ${result_warning_guard}
        INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(result INTERFACE -stdlib=libc++)
elseif(NOT MSVC)
  message(FATAL_ERROR "Compiler not supported: ${CMAKE_CXX_COMPILER_ID}")
endif()

# ---- Install ----

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(result_directory "result-${PROJECT_VERSION}")
set(result_include_directory "${CMAKE_INSTALL_INCLUDEDIR}/${result_directory}")

install(TARGETS result
        EXPORT resultTargets
        INCLUDES DESTINATION "${result_include_directory}")

set(result_install_cmakedir
        "${CMAKE_INSTALL_LIBDIR}/cmake/${result_directory}")

write_basic_package_version_file(
        resultConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMinorVersion
        ARCH_INDEPENDENT)

install(EXPORT resultTargets
        FILE resultConfig.cmake # Install targets directly as the config
        NAMESPACE DimanNe::
        DESTINATION "${result_install_cmakedir}")

install(FILES
        "${PROJECT_BINARY_DIR}/resultConfigVersion.cmake"
        DESTINATION "${result_install_cmakedir}")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
        DESTINATION "${result_include_directory}")
