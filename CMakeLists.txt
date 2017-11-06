cmake_minimum_required (VERSION 3.4)

# Set CMAKE_BUILD_TYPE to Release by default.
if (DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    )
else ()
  set (CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    )
endif ()

project(cmpchk CXX)

SET(CMAKE_C_COMPILER /usr/bin/gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -O2")

add_library (cmpchk SHARED src/ComplexityProfiler.cpp)
target_include_directories(cmpchk PUBLIC inc)

add_executable (cmpchk_test ../test/ComplexityProfiler_test.cpp)
target_link_libraries (cmpchk_test LINK_PUBLIC cmpchk)

#INSTALL(TARGETS unarray LIBRARY DESTINATION lib)
#INSTALL(FILES unarray.h DESTINATION include)
