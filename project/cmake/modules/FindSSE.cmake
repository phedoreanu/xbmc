# Check if SSE instructions are available on the machine where 
# the project is compiled.

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
   exec_program(cat ARGS "/proc/cpuinfo" OUTPUT_VARIABLE CPUINFO)

   string(REGEX REPLACE "^.*(sse2).*$" "\\1" SSE_THERE ${CPUINFO})
   string(COMPARE EQUAL "sse2" "${SSE_THERE}" SSE2_TRUE)
   if(SSE2_TRUE)
      set(SSE2_FOUND true CACHE BOOL "SSE2 available on host")
   else()
      set(SSE2_FOUND false CACHE BOOL "SSE2 available on host")
   endif()

   # /proc/cpuinfo apparently omits sse3 :(
   string(REGEX REPLACE "^.*[^s](sse3).*$" "\\1" SSE_THERE ${CPUINFO})
   string(COMPARE EQUAL "sse3" "${SSE_THERE}" SSE3_TRUE)
   if(NOT SSE3_TRUE)
      string(REGEX REPLACE "^.*(T2300).*$" "\\1" SSE_THERE ${CPUINFO})
      string(COMPARE EQUAL "T2300" "${SSE_THERE}" SSE3_TRUE)
   endif()

   string(REGEX REPLACE "^.*(ssse3).*$" "\\1" SSE_THERE ${CPUINFO})
   string(COMPARE EQUAL "ssse3" "${SSE_THERE}" SSSE3_TRUE)
   if(SSE3_TRUE OR SSSE3_TRUE)
      set(SSE3_FOUND true CACHE BOOL "SSE3 available on host")
   else()
      set(SSE3_FOUND false CACHE BOOL "SSE3 available on host")
   endif()
   if(SSSE3_TRUE)
      set(SSSE3_FOUND true CACHE BOOL "SSSE3 available on host")
   else()
      set(SSSE3_FOUND false CACHE BOOL "SSSE3 available on host")
   endif()

   string(REGEX REPLACE "^.*(sse4_1).*$" "\\1" SSE_THERE ${CPUINFO})
   string(COMPARE EQUAL "sse4_1" "${SSE_THERE}" SSE41_TRUE)
   if(SSE41_TRUE)
      set(SSE4_1_FOUND true CACHE BOOL "SSE4.1 available on host")
   else()
      set(SSE4_1_FOUND false CACHE BOOL "SSE4.1 available on host")
   endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   exec_program("/usr/sbin/sysctl -n machdep.cpu.features" OUTPUT_VARIABLE
      CPUINFO)

   string(REGEX REPLACE "^.*[^S](SSE2).*$" "\\1" SSE_THERE ${CPUINFO})
   string(COMPARE EQUAL "SSE2" "${SSE_THERE}" SSE2_TRUE)
   if(SSE2_TRUE)
      set(SSE2_FOUND true CACHE BOOL "SSE2 available on host")
   else()
      set(SSE2_FOUND false CACHE BOOL "SSE2 available on host")
   endif()

   string(REGEX REPLACE "^.*[^S](SSE3).*$" "\\1" SSE_THERE ${CPUINFO})
   string(COMPARE EQUAL "SSE3" "${SSE_THERE}" SSE3_TRUE)
   if(SSE3_TRUE)
      set(SSE3_FOUND true CACHE BOOL "SSE3 available on host")
   else()
      set(SSE3_FOUND false CACHE BOOL "SSE3 available on host")
   endif()

   string(REGEX REPLACE "^.*(SSSE3).*$" "\\1" SSE_THERE ${CPUINFO})
   string(COMPARE EQUAL "SSSE3" "${SSE_THERE}" SSSE3_TRUE)
   if(SSSE3_TRUE)
      set(SSSE3_FOUND true CACHE BOOL "SSSE3 available on host")
   else()
      set(SSSE3_FOUND false CACHE BOOL "SSSE3 available on host")
   endif()

   string(REGEX REPLACE "^.*(SSE4.1).*$" "\\1" SSE_THERE ${CPUINFO})
   string(COMPARE EQUAL "SSE4.1" "${SSE_THERE}" SSE41_TRUE)
   if(SSE41_TRUE)
      set(SSE4_1_FOUND true CACHE BOOL "SSE4.1 available on host")
   else()
      set(SSE4_1_FOUND false CACHE BOOL "SSE4.1 available on host")
   endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   # TODO
   set(SSE2_FOUND   true  CACHE BOOL "SSE2 available on host")
   set(SSE3_FOUND   false CACHE BOOL "SSE3 available on host")
   set(SSSE3_FOUND  false CACHE BOOL "SSSE3 available on host")
   set(SSE4_1_FOUND false CACHE BOOL "SSE4.1 available on host")
else()
   set(SSE2_FOUND   true  CACHE BOOL "SSE2 available on host")
   set(SSE3_FOUND   false CACHE BOOL "SSE3 available on host")
   set(SSSE3_FOUND  false CACHE BOOL "SSSE3 available on host")
   set(SSE4_1_FOUND false CACHE BOOL "SSE4.1 available on host")
endif()

if(NOT SSE2_FOUND)
      message(STATUS "Could not find hardware support for SSE2 on this machine.")
endif()
if(NOT SSE3_FOUND)
      message(STATUS "Could not find hardware support for SSE3 on this machine.")
endif()
if(NOT SSSE3_FOUND)
      message(STATUS "Could not find hardware support for SSSE3 on this machine.")
endif()
if(NOT SSE4_1_FOUND)
      message(STATUS "Could not find hardware support for SSE4.1 on this machine.")
endif()

mark_as_advanced(SSE2_FOUND SSE3_FOUND SSSE3_FOUND SSE4_1_FOUND)

