set(Project diresult)
add_library(${Project} STATIC
   result.h
   result.cpp
   result_with_using.cpp
   result_with_using.h
   coresult.h
   coresult.cpp
   coresult_with_using.h
   coresult_with_using.cpp
)
