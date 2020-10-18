cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(test_result_as_subproject)

add_subdirectory(cxxabi) # Here we build a very low-level library (std::)

# Then specify it for all project of this (fake) monorepo
link_libraries(cxxabi)

# And sooner or later somebody wants to use the Result<> library:
add_subdirectory(../../ ./result_binary_dir)
# ... which results in errors like:
# CMake Error: install(EXPORT "resultTargets" ...) includes target "result" which requires target "cxxabi" that is not in any export set.
# CMake Error: install(EXPORT "GTestTargets" ...) includes target "gtest" which requires target "cxxabi" that is not in any export set.
# CMake Error: install(EXPORT "GTestTargets" ...) includes target "gtest_main" which requires target "cxxabi" that is not in any export set.
# CMake Error: install(EXPORT "GTestTargets" ...) includes target "gmock" which requires target "cxxabi" that is not in any export set.
# CMake Error: install(EXPORT "GTestTargets" ...) includes target "gmock_main" which requires target "cxxabi" that is not in any export set.