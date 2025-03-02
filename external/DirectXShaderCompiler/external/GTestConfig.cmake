########################################################################
# Experimental CMake build script for Google Test.
#
# Consider this a prototype.  It will change drastically.  For now,
# this is only for people on the cutting edge.
#
# To run the tests for Google Test itself on Linux, use 'make test' or
# ctest.  You can select which tests to run using 'ctest -R regex'.
# For more options, run 'ctest --help'.
########################################################################
#
# Project-wide settings

# Where gtest's .h files can be found.
include_directories(
  ${DXC_GTEST_DIR}/googletest/include
  ${DXC_GTEST_DIR}/googletest
  ${DXC_GTEST_DIR}/googlemock/include
  ${DXC_GTEST_DIR}/googlemock
  )

if(WIN32)
  add_definitions(-DGTEST_OS_WINDOWS=1)
else(WIN32)
  # Disable all warnings in subproject googletest
  add_compile_options(-w)
endif(WIN32)

set(LLVM_REQUIRES_RTTI 1)
add_definitions( -DGTEST_HAS_RTTI=0 )

if (NOT LLVM_ENABLE_THREADS)
  add_definitions( -DGTEST_HAS_PTHREAD=0 )
endif()

find_library(LLVM_PTHREAD_LIBRARY_PATH pthread)
if (LLVM_PTHREAD_LIBRARY_PATH)
  list(APPEND LIBS pthread)
endif()

add_llvm_library(gtest
  ${DXC_GTEST_DIR}/googletest/src/gtest-all.cc
  ${DXC_GTEST_DIR}/googlemock/src/gmock-all.cc
)