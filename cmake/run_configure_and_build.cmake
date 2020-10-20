# Don't ignore empty list elements
cmake_policy(SET CMP0007 NEW)

# ------------------- Parse args -------------------
set(args "")
foreach(n RANGE ${CMAKE_ARGC})
  if(NOT "${CMAKE_ARGV${n}}" STREQUAL "")
    list(APPEND args "${CMAKE_ARGV${n}}")
  endif()
endforeach()

list(FIND args "--" index)
if(index EQUAL -1)
  message(FATAL_ERROR "No -- divider found in arguments list")
else()
  set(temp "${args}")
  math(EXPR index "${index} + 1")
  list(SUBLIST temp ${index} -1 args)
endif()

list(POP_FRONT args source sanitiser_mode cores)


# ------------------- Define exec function -------------------
function(exec)
  set(args "")
  foreach(arg IN LISTS ARGN)
    string(FIND "${arg}" " " index)
    if(index EQUAL -1)
      list(APPEND args "${arg}")
    else()
      list(APPEND args "\"${arg}\"")
    endif()
  endforeach()

  string(ASCII 27 Esc)
  list(JOIN args " " args)
  execute_process(COMMAND "${CMAKE_COMMAND}" -E echo "${Esc}[36mExecuting: ${args}${Esc}[m")

  execute_process(COMMAND ${ARGN} RESULT_VARIABLE result)

  if(NOT result EQUAL 0)
    message(FATAL_ERROR "${Esc}[1;31mBad exit status (${result})${Esc}[m")
  endif()
endfunction()


# ------------------- Magic for windows -------------------
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
   execute_process(
      COMMAND "C:/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Auxiliary/Build/vcvars64.bat" && set
      OUTPUT_FILE environment_script_output.txt
   )
   file(STRINGS environment_script_output.txt output_lines)
   foreach(line IN LISTS output_lines)
      if(line MATCHES "^([a-zA-Z0-9_-]+)=(.*)$")
         set(ENV{${CMAKE_MATCH_1}} "${CMAKE_MATCH_2}")
      endif()
   endforeach()
endif()


# ------------------- Configure & Build -------------------
exec("${CMAKE_COMMAND}" -S "${source}" -B build -D RESULT_IS_BUILT_AS_A_PART_OF_MONOREPO=OFF -D sanitiser_mode=${sanitiser_mode} -D CMAKE_INSTALL_PREFIX=build/prefix)
exec("${CMAKE_COMMAND}" --build build --verbose -j ${cores})
