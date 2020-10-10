set(Project diresult_ut)

add_executable(${Project}
   result.t.cpp
   # coresult.t.cpp
)

# ============= GTest =============
target_link_libraries(${Project} PRIVATE diresult)
target_link_libraries(${Project} PRIVATE gtest gmock gtest_main)
add_test(diresult_tests ${Project})
