set(Project diresult_example)
add_executable(${Project}
   main.cpp
)
target_link_libraries(${Project} PRIVATE diresult)
