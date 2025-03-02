# Toolchain config for iOS.
#
# Usage:
# mkdir build; cd build
# cmake ..; make
# mkdir ios; cd ios
# cmake -DLLVM_IOS_TOOLCHAIN_DIR=/path/to/ios/ndk \
#   -DCMAKE_TOOLCHAIN_FILE=../../cmake/platforms/iOS.cmake ../..
# make <target>

SET(CMAKE_SYSTEM_NAME Darwin)
SET(CMAKE_SYSTEM_VERSION 13)
SET(CMAKE_CXX_COMPILER_WORKS True)
SET(CMAKE_C_COMPILER_WORKS True)
SET(DARWIN_TARGET_OS_NAME ios)

IF(NOT DEFINED ENV{SDKROOT})
 execute_process(COMMAND xcodebuild -version -sdk iphoneos Path
   OUTPUT_VARIABLE SDKROOT
   ERROR_QUIET
   OUTPUT_STRIP_TRAILING_WHITESPACE)
ELSE()
  execute_process(COMMAND xcodebuild -version -sdk $ENV{SDKROOT} Path
   OUTPUT_VARIABLE SDKROOT
   ERROR_QUIET
   OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF()

IF(NOT EXISTS ${SDKROOT})
  MESSAGE(FATAL_ERROR "SDKROOT could not be detected!")
ENDIF()

set(CMAKE_OSX_SYSROOT ${SDKROOT})

IF(NOT CMAKE_C_COMPILER)
  execute_process(COMMAND xcrun -sdk ${SDKROOT} -find clang
   OUTPUT_VARIABLE CMAKE_C_COMPILER
   ERROR_QUIET
   OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Using c compiler ${CMAKE_C_COMPILER}")
ENDIF()

IF(NOT CMAKE_CXX_COMPILER)
  execute_process(COMMAND xcrun -sdk ${SDKROOT} -find clang++
   OUTPUT_VARIABLE CMAKE_CXX_COMPILER
   ERROR_QUIET
   OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Using c compiler ${CMAKE_CXX_COMPILER}")
ENDIF()

IF(NOT CMAKE_AR)
  execute_process(COMMAND xcrun -sdk ${SDKROOT} -find ar
   OUTPUT_VARIABLE CMAKE_AR_val
   ERROR_QUIET
   OUTPUT_STRIP_TRAILING_WHITESPACE)
  SET(CMAKE_AR ${CMAKE_AR_val} CACHE FILEPATH "Archiver")
  message(STATUS "Using ar ${CMAKE_AR}")
ENDIF()

IF(NOT CMAKE_RANLIB)
  execute_process(COMMAND xcrun -sdk ${SDKROOT} -find ranlib
   OUTPUT_VARIABLE CMAKE_RANLIB_val
   ERROR_QUIET
   OUTPUT_STRIP_TRAILING_WHITESPACE)
  SET(CMAKE_RANLIB ${CMAKE_RANLIB_val} CACHE FILEPATH "Ranlib")
  message(STATUS "Using ranlib ${CMAKE_RANLIB}")
ENDIF()

IF (NOT DEFINED IOS_MIN_TARGET)
execute_process(COMMAND xcodebuild -sdk ${SDKROOT} -version SDKVersion
   OUTPUT_VARIABLE IOS_MIN_TARGET
   ERROR_QUIET
   OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF()

SET(IOS_COMMON_FLAGS "-mios-version-min=${IOS_MIN_TARGET}")
SET(CMAKE_C_FLAGS "${IOS_COMMON_FLAGS}" CACHE STRING "toolchain_cflags" FORCE)
SET(CMAKE_CXX_FLAGS "${IOS_COMMON_FLAGS}" CACHE STRING "toolchain_cxxflags" FORCE)
SET(CMAKE_LINK_FLAGS "${IOS_COMMON_FLAGS}" CACHE STRING "toolchain_linkflags" FORCE)
