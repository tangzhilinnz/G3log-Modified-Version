# =============================================================================
# 2010 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
# with no warranties. This code is yours to share, use and modify with no
# strings attached and no restrictions or obligations.
#
# For more information see g3log/LICENSE or refer refer to http://unlicense.org
# ==============================================================================

# Below are details for compiling on Windows and Linux by default only an
# example g3log binary is created the performance and unit tests creation can be
# enabled by switching their OPTIONs from OFF to ON --- See below at around line
# 110

# === WINDOWS ===
# Example for: Visual Studio 2013 (earlier should work too) 1. please use the
# "Visual Studio Command Prompt 12 (2013)" 2. from the g3log folder mkdir build
# cd build; 3. cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio XXX" .. (cmake
# -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 12") MAKE SURE you check the
# CMake documentation so you are using the correct bit flags(64 bit etc). The
# "XXX" needs tto be replaced for your specific build system, ref: cmake docs.
#
# (Example from Appveyor Ci:
# https://github.com/KjellKod/g3log/blob/master/appveyor.yml cmake -G "Visual
# Studio 14 2015 Win64" -DADD_G3LOG_UNIT_TEST=ON ..)
#
# 1. msbuild g3log.sln /p:Configuration=Release
#
# Try to run an example, such as: 5. Release\g3log-FATAL-contract.exe
#

# ===   LINUX: === To try this out from folder g3log: mkdir build cd build >>
# create makefiles in g3log/build directory cmake -DCMAKE_BUILD_TYPE=Release ..
# make -jN   (where N stands for number of cores you want to utilize)
#
# ===  Clang on Linux ===
# From g3log mkdir build && cd build cmake -DCMAKE_CXX_COMPILER=clang++ .. if
# you want to double-check settings:   "VERBOSE=1 make" otherwise just run:
# "make -j"
#
# ============================================================================

cmake_minimum_required(VERSION 3.2)

project(g3log CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT (MSVC_IDE OR XCODE))
  # set(<variable> <value>... CACHE STRING <docstring> [FORCE])
  # Sets the given cache <variable> (cache entry). Since cache entries are 
  # meant to provide user-settable values this does not overwrite existing
  # cache entries by default. 
  # FORCE: Use the FORCE option to overwrite existing entries.
  # STRING: A line of text. cmake-gui offers a text field or a drop-down 
  # selection if the STRINGS cache entry property is set.
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Build type, one of: Release, Debug" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
# CMAKE_CONFIGURATION_TYPES
# This specifies what build types (configurations) will be available such as 
# Debug, Release, RelWithDebInfo etc. This has reasonable defaults on most 
# platforms, but can be extended to provide other build types.
message(STATUS "Configuration types: ${CMAKE_CONFIGURATION_TYPES}")

# Detect 64 or 32 bit 
# CMAKE_SIZEOF_VOID_P
# This is set to the size of a pointer on the target machine, and is determined
# by a try compile. If a 64-bit size is found, then the library search path is 
# modified to look for 64-bit libraries first.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  # 64-bit project
  set(64_BIT_OS TRUE)
  message(STATUS "A 64-bit OS detected")
else()
  set(64_BIT_OS FALSE)
  message(STATUS "A 32-bit OS detected")
endif()


# Calculate the version number
SET(MAJOR_VERSION 1)
SET(MINOR_VERSION 3)

# CMAKE_SYSTEM_NAME is by default set to the same value as the 
# CMAKE_HOST_SYSTEM_NAME variable so that the build targets the host system.
# CMAKE_CURRENT_SOURCE_DIR
# This is the full path to the source directory that is currently being 
# processed by cmake.
IF ( NOT VERSION )
   IF ( "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows" )
      message("windows: Extracting git software version")
      # cmd /c "<string>" Carries out the command specified by the string
      # and then terminates.
      execute_process(COMMAND cmd /c "git rev-list --branches HEAD | find /v " " /c" OUTPUT_VARIABLE GIT_VERSION WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
   ELSE()
      IF(UNIX OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
         message( STATUS "nix: Extracting git software version" )
      ELSE()
         message( STATUS "unknown platform: extracting git software version" )
      ENDIF()
      # bash "-c" "<string>" If the -c option is present, then commands are
      # read from string. 
      # If there are arguments after the string, they are assigned to the
      # positional parameters, starting with $0.
      execute_process(COMMAND bash "-c" "git rev-list --branches HEAD | wc -l | tr -d ' ' | tr -d '\n'" OUTPUT_VARIABLE GIT_VERSION WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
   ENDIF()

   # math(EXPR <output variable> <math expression>)
   # EXPR evaluates mathematical expression and returns result in the output variable.
   math(EXPR VERSION-BASE ${GIT_VERSION}/255)
   math(EXPR VERSION-REMAINDER ${GIT_VERSION}%255)
   message( STATUS "git build version: ${GIT_VERSION}" )
   message( STATUS "version base: ${VERSION-BASE}" )
   message( STATUS "version remainder: ${VERSION-REMAINDER}" )
   SET(BUILD_NUMBER ${VERSION-BASE})
   SET(VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_NUMBER}-${VERSION-REMAINDER})
ENDIF()
message( STATUS "Software Version: ${VERSION}" )

# ============================================================================
# G3LOG OPTIONAL FEATURES
# ============================================================================
include(${g3log_SOURCE_DIR}/Options.cmake)

# ============================================================================
# G3LOG iOS BUILD SUPPORT
# ============================================================================
include(${g3log_SOURCE_DIR}/iOSBuild.cmake)

if(G3_IOS_LIB)
  # G3_IOS_LIB is the pass used to generate all the other cmakefiles for the
  # different architectures needed for the universal library. So we're done at
  # here.
  return()
endif()

# =========================================================================
# G3 Macro definitions in Options.cmake are written to file this avoids having
# to re-state your definitions in your source code or compile options
# ==========================================================================
include(${g3log_SOURCE_DIR}/GenerateMacroDefinitionsFile.cmake)

# =========================================================================
# G3LOG BUILD
# ==========================================================================
include(${g3log_SOURCE_DIR}/Build.cmake)

# ============================================================================
# EXAMPLE OPTIONS: By defauls is ON. This will create 'g3log-FATAL-* examples'
# ============================================================================
# DISABLE WITH:  -DADD_FATAL_EXAMPLE=OFF
include(${g3log_SOURCE_DIR}/example/Example.cmake)

# ============================================================================
# PERFORMANCE TEST OPTIONS: Performance operations for g3log
# ============================================================================
# ENABLE WITH:  -DADD_G3LOG_PERFORMANCE=ON
include(${g3log_SOURCE_DIR}/test_performance/Performance.cmake)

# ==========================================================================
# UNIT TEST OPTIONS:
# ============================================================================
# ENABLE WITH:  -DADD_G3LOG_UNIT_TEST=ON
include(${g3log_SOURCE_DIR}/test_unit/Test.cmake)

# ==========================================================================
# CMAKE INSTALL AND CPACK OPTIONS:
# ==========================================================================
#
# Alternative 1: Package handling is done AFTER all other CMake setup usage:
# make package Check the output result and install accordingly.
#
# Alternative 2: usage: make; sudo make install
#
# For OSX you can also install an older version using 'brew install'
#
# ==========================================================================
include(${g3log_SOURCE_DIR}/CPackLists.txt)

if(MINGW)
  # this enables strerror_s
  # add_definitions(-DFOO -DBAR ...)
  # Adds definitions to the compiler command line for targets in the current 
  # directory and below. This command can be used to add any flags, but it is
  # intended to add preprocessor definitions
  add_definitions(-DMINGW_HAS_SECURE_API)
endif()

if(NOT MSVC)
  message(
    STATUS
      "\n\n
      *******************************************************************
      Please do 'make clean-cmake' before next cmake generation.
         It is a good idea to purge your build directory of CMake
         generated cache files
      *******************************************************************
  ")
  # CMAKE_COMMAND
  # This is the full path to the CMake executable cmake which is useful 
  # from custom commands that want to use the cmake -E option for portable 
  # system commands. (e.g. /usr/local/bin/cmake)
  # add_custom_target
  # Adds a target with the given name that executes the given commands.
  add_custom_target(clean-cmake COMMAND ${CMAKE_COMMAND} -P
                                        ${g3log_SOURCE_DIR}/CleanAll.cmake)
endif()
