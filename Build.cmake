# g3log is a KjellKod Logger
# 2015 @author Kjell Hedström, hedstrom@kjellkod.cc
# ==================================================================
# 2015 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own
#    risk and comes  with no warranties.
#
# This code is yours to share, use and modify with no strings attached
#   and no restrictions or obligations.
# ===================================================================

# GENERIC STEPS
SET(LOG_SRC ${g3log_SOURCE_DIR}/src)

# file(GLOB <variable>
#     [LIST_DIRECTORIES true|false] [RELATIVE <path>] [CONFIGURE_DEPENDS]
#     [<globbing-expressions>...])
# Generate a list of files that match the <globbing-expressions> and store it into the <variable>. 
file(GLOB SRC_FILES  ${LOG_SRC}/*.cpp ${LOG_SRC}/*.ipp)
file(GLOB HEADER_FILES ${LOG_SRC}/g3log/*.hpp)

list( APPEND HEADER_FILES ${GENERATED_G3_DEFINITIONS} )
list( APPEND SRC_FILES ${GENERATED_G3_DEFINITIONS} )

IF (MSVC OR MINGW)
   list(REMOVE_ITEM SRC_FILES  ${LOG_SRC}/crashhandler_unix.cpp)
ELSE()
   # list(REMOVE_ITEM SRC_FILES  ${LOG_SRC}/crashhandler_windows.cpp ${LOG_SRC}/g3log/stacktrace_windows.hpp ${LOG_SRC}/stacktrace_windows.cpp)
   list(REMOVE_ITEM SRC_FILES  ${LOG_SRC}/crashhandler_windows.cpp  ${LOG_SRC}/stacktrace_windows.cpp)
   list(REMOVE_ITEM HEADER_FILES  ${LOG_SRC}/g3log/stacktrace_windows.hpp)

ENDIF (MSVC OR MINGW)

set(SRC_FILES ${SRC_FILES} ${SRC_PLATFORM_SPECIFIC})

# Create the g3log library
SET(G3LOG_LIBRARY g3log)


IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # CMAKE_INSTALL_PREFIX
    # If make install is invoked or INSTALL is built, this directory is 
    # prepended onto all install directories. This variable defaults to 
    # /usr/local on UNIX and c:/Program Files/${PROJECT_NAME} on Windows.
    message("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
    IF( NOT CMAKE_INSTALL_PREFIX)
       SET(CMAKE_INSTALL_PREFIX /usr/local)
    ENDIF()

    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    message("Install rpath location: ${CMAKE_INSTALL_RPATH}")
ENDIF()

IF( G3_SHARED_LIB )
   IF( WIN32 )
      # Use the if() command VERSION_LESS, VERSION_GREATER, VERSION_EQUAL, 
      # VERSION_LESS_EQUAL, or VERSION_GREATER_EQUAL operators to compare 
      # version string values against CMAKE_VERSION using a component-wise test.
      IF(NOT(${CMAKE_VERSION} VERSION_LESS "3.4"))
         # WINDOWS_EXPORT_ALL_SYMBOLS property
         # This property is implemented only for MS-compatible tools on Windows.
         # This property is initialized by the value of the CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 
         # variable if it is set when a target is created.
         set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
      ELSE()
         # FATAL_ERROR: CMake Error, stop processing and generation.
         message( FATAL_ERROR "Need CMake version >=3.4 to build shared windows library!" )
      ENDIF()
   ENDIF()
   ADD_LIBRARY(${G3LOG_LIBRARY} SHARED ${SRC_FILES})
ELSE()
   IF(MSVC)
      IF(NOT G3_SHARED_RUNTIME)
         SET(CompilerFlags
               CMAKE_CXX_FLAGS
               CMAKE_CXX_FLAGS_DEBUG
               CMAKE_CXX_FLAGS_RELEASE
               CMAKE_C_FLAGS
               CMAKE_C_FLAGS_DEBUG
               CMAKE_C_FLAGS_RELEASE
            )
         foreach(CompilerFlag ${CompilerFlags})
            # string(REPLACE <match_string> <replace_string> 
            #        <output_variable> <input> [<input>...])
            # Replace all occurrences of <match_string> in the <input> with 
            # <replace_string> and store the result in the <output_variable>.
            string(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
         endforeach()
      ENDIF()
   ENDIF()
   ADD_LIBRARY(${G3LOG_LIBRARY} STATIC ${SRC_FILES})
ENDIF()

SET(${G3LOG_LIBRARY}_VERSION_STRING ${VERSION})
MESSAGE( STATUS "Creating ${G3LOG_LIBRARY} VERSION: ${VERSION}" )
MESSAGE( STATUS "Creating ${G3LOG_LIBRARY} SOVERSION: ${MAJOR_VERSION}" )

SET_TARGET_PROPERTIES(${G3LOG_LIBRARY} PROPERTIES
   LINKER_LANGUAGE CXX
   OUTPUT_NAME g3log
   CLEAN_DIRECT_OUTPUT 1
   SOVERSION ${MAJOR_VERSION}
   VERSION ${VERSION}
)


# APPLE: Set to True when the target system is an Apple platform
# (macOS, iOS, tvOS or watchOS).
# MACOSX_RPATH TRUE
# This indicates the shared library is to be found at runtime using 
# runtime paths (rpaths).
IF(APPLE)
   SET_TARGET_PROPERTIES(${G3LOG_LIBRARY} PROPERTIES MACOSX_RPATH TRUE)
ENDIF()

# require here some proxy for c++14 standard to avoid problems TARGET_PROPERTY CXX_STANDARD
TARGET_COMPILE_FEATURES(${G3LOG_LIBRARY} PUBLIC cxx_variable_templates)

# $<BUILD_INTERFACE:...>
# Content of ... when the property is exported using export(), or when the 
# target is used by another target in the same buildsystem. Expands to the
# empty string otherwise.
TARGET_INCLUDE_DIRECTORIES(${G3LOG_LIBRARY}
   PUBLIC
      $<BUILD_INTERFACE:${LOG_SRC}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)

SET(ACTIVE_CPP0xx_DIR "Release")

# find corresponding thread lib (e.g. whether -lpthread is needed or not)
FIND_PACKAGE(Threads REQUIRED)
TARGET_LINK_LIBRARIES(${G3LOG_LIBRARY} Threads::Threads )

# check for backtrace and cxa_demangle only in non-Windows dev environments
IF(NOT(MSVC OR MINGW))
   # the backtrace module does not provide a modern cmake target
   FIND_PACKAGE(Backtrace REQUIRED)
   # Backtrace_FOUND: Is set if and only if backtrace support detected.
   # Backtrace_INCLUDE_DIRS: The include directories needed to use backtrace header.
   # Backtrace_LIBRARIES: The libraries (linker flags) needed to use backtrace, if any.
   if(Backtrace_FOUND)
      TARGET_INCLUDE_DIRECTORIES(${G3LOG_LIBRARY} PRIVATE ${Backtrace_INCLUDE_DIRS})
      TARGET_LINK_LIBRARIES(${G3LOG_LIBRARY} ${Backtrace_LIBRARIES})
   else()
      message( FATAL_ERROR "Could not find Library to create backtraces")
   endif()

   INCLUDE(CheckLibraryExists)
   INCLUDE(CheckCXXSymbolExists)

   #if demangle is in c++ runtime lib
   # CHECK_CXX_SYMBOL_EXISTS( <symbol> <files> <variable> )
   # Check that the <symbol> is available after including the given header <files>
   # and store the result in a <variable>.
   # CHECK_LIBRARY_EXISTS( <LIBRARY> <FUNCTION> <LOCATION> <VARIABLE> )
   #   LIBRARY  - the name of the library you are looking for
   #   FUNCTION - the name of the function
   #   LOCATION - location where the library should be found
   #   VARIABLE - variable to store the result that will be created as an
   #              internal cache variable.
   CHECK_CXX_SYMBOL_EXISTS(abi::__cxa_demangle "cxxabi.h" DEMANGLE_EXISTS)
   IF( NOT (DEMANGLE_EXISTS))
      #try to link against c++abi to get demangle
      CHECK_LIBRARY_EXISTS(c++abi abi::__cxa_demangle "cxxabi.h" NEED_C++ABI)
      IF( NEED_C++ABI)
         TARGET_LINK_LIBRARIES(${G3LOG_LIBRARY} c++abi)
      ELSE()
         message( FATAL_ERROR "Could not find function abi::__cxa_demangle")
      ENDIF()

   ENDIF()
ENDIF()

# add Warnings
target_compile_options(${G3LOG_LIBRARY} PRIVATE
   # clang/GCC warnings
   $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wunused>
   # MSVC warnings
   $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# add GCC specific stuff
target_compile_options(${G3LOG_LIBRARY} PRIVATE
   # clang/GCC warnings
   $<$<AND:$<CXX_COMPILER_ID:GNU>,$<NOT:$<BOOL:${MINGW}>>>:-rdynamic>
)

#cmake -DCMAKE_CXX_COMPILER=clang++ ..
  # WARNING: If Clang for Linux does not work with full c++14 support it might be your
  # installation that is faulty. When I tested Clang on Ubuntu I followed the following
  # description
  #  1) http://kjellkod.wordpress.com/2013/09/23/experimental-g3log-with-clang/
  #  2) https://github.com/maidsafe/MaidSafe/wiki/Hacking-with-Clang-llvm-abi-and-llvm-libc

# Windows Stuff
IF(MSVC OR MINGW)
   TARGET_COMPILE_DEFINITIONS(${G3LOG_LIBRARY} PRIVATE NOGDI)
   TARGET_LINK_LIBRARIES(${G3LOG_LIBRARY} dbghelp)
   # VC11 bug: http://code.google.com/p/googletest/issues/detail?id=408
   #          add_definition(-D_VARIADIC_MAX=10)
   # https://github.com/anhstudios/swganh/pull/186/files
   TARGET_COMPILE_DEFINITIONS(${G3LOG_LIBRARY} PRIVATE _VARIADIC_MAX=10)
   MESSAGE(STATUS "- MSVC: Set variadic max to 10 for MSVC compatibility")
   # Remember to set set target properties if using GTEST similar to done below on target "unit_test"
   # "set_target_properties(unit_test  PROPERTIES COMPILE_DEFINITIONS "GTEST_USE_OWN_TR1_TUPLE=0")
   message( STATUS "" )
   message( STATUS "Windows: Run cmake with the appropriate Visual Studio generator" )
   message( STATUS "The generator is one number below the official version number. I.e. VS2013 -> Generator 'Visual Studio 12'" )
   MESSAGE( STATUS "I.e. if VS2013: Please run the command [cmake -DCMAKE_BUILD_TYPE=Release -G \"Visual Studio 12\" ..]")
   message( STATUS "if cmake finishes OK, do 'msbuild g3log.sln /p:Configuration=Release'" )
   message( STATUS "then run 'Release\\g3log-FATAL-*' examples" )
   message( STATUS "" )
ENDIF()

TARGET_COMPILE_OPTIONS(${G3LOG_LIBRARY} PRIVATE
   $<$<CXX_COMPILER_ID:MSVC>:/utf-8> # source code already in utf-8, force it for compilers in non-utf8_windows_locale
   $<$<CXX_COMPILER_ID:MSVC>:$<$<EQUAL:4,${CMAKE_SIZEOF_VOID_P}>:/arch:IA32>>
)

# copy .hpp files from src to build directory (include) for the convenient including by other targets
foreach(CurrentHeaderFile IN LISTS HEADER_FILES)
   add_custom_command(
      TARGET ${G3LOG_LIBRARY} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CurrentHeaderFile} "${CMAKE_CURRENT_BINARY_DIR}/include/g3log/"
      COMMENT "Copying header: ${CurrentHeaderFile}")
endforeach()
