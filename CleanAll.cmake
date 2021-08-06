# ==========================================================================
# 2015 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
# with no warranties. This code is yours to share, use and modify with no
# strings attached and no restrictions or obligations.
# 
# For more information see g3log/LICENSE or refer refer to http://unlicense.org
# ============================================================================*/

# CMAKE_BINARY_DIR
# This is the full path to the top level of the current CMake build tree. For 
# an in-source build, this would be the same as CMAKE_SOURCE_DIR.
set(cmake_generated ${CMAKE_BINARY_DIR}/CMakeCache.txt
                    ${CMAKE_BINARY_DIR}/cmake_install.cmake  
                    ${CMAKE_BINARY_DIR}/Makefile
                    ${CMAKE_BINARY_DIR}/CMakeFiles
)

foreach(file ${cmake_generated})
  if (EXISTS ${file})
     message( STATUS "Removing: ${file}" )
     # file(REMOVE_RECURSE [<files>...])
     # Remove the given files. The REMOVE_RECURSE mode will remove the given 
     # files and directories, also non-empty directories. No error is emitted 
     # if a given file does not exist. 
     file(REMOVE_RECURSE ${file})
  endif()
endforeach(file)