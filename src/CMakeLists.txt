set(Project diresult)
add_library(${Project} STATIC
   result.h
   result.cpp
   result_with_using.cpp
   result_with_using.h
)
