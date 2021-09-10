#.rst:
# FindG3log
# -------
#
# Find libg3log, G3log is an asynchronous, "crash safe", logger that is easy to use with default logging sinks or you can add your own.
#
# This defines the cmake import target "g3log" you can use like this
#```
# target_link_libraries(YourTarget PUBLIC g3log)
#```
# Variables and features 
# ----------------------
# * ``G3LOG`` -- if this environment variable is set, it'll be used as a hint as to where the g3log files are. 
# * ``G3LOG_INCLUDE_DIRS`` -- raw cmake variable with include path 
# * ``G3LOG_LIBRARIES`` -- raw cmake variable with library link line
# * ``G3LOG_FOUND`` -- check if the lib was found without using the newer ``if(TARGET g3log)...``

# FindPackageHandleStandardArgs
# This module provides functions intended to be used in Find Modules implementing
# find_package(<PackageName>) calls.
include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

@PACKAGE_INIT@

find_package(Threads REQUIRED)

if (NOT TARGET g3log)
   include("${CMAKE_CURRENT_LIST_DIR}/g3logTargets.cmake")
   
   # get_target_property(<VAR> target property)
   # Get a property from a target. The value of the property is stored in the 
   # variable <VAR>.
   # INTERFACE_INCLUDE_DIRECTORIES
   # List of public include directories requirements for a library.
   # 
   get_target_property(G3LOG_INCLUDE_DIR g3log INTERFACE_INCLUDE_DIRECTORIES)
   get_target_property(G3LOG_LIBRARY_DEBUG g3log IMPORTED_IMPLIB_DEBUG)
   
   if (G3LOG_LIBRARY_DEBUG MATCHES ".*-NOTFOUND")
      get_target_property(G3LOG_LIBRARY_DEBUG g3log IMPORTED_LOCATION_DEBUG)
   endif ()

   get_target_property(G3LOG_LIBRARY_RELEASE g3log IMPORTED_IMPLIB_RELEASE)
   if (G3LOG_LIBRARY_RELEASE MATCHES ".*-NOTFOUND")
      get_target_property(G3LOG_LIBRARY_RELEASE g3log IMPORTED_LOCATION_RELEASE)
   endif ()

   # select_library_configurations(basename)
   # This macro takes a library base name as an argument, and will choose good
   # values for the variables
   # basename_LIBRARY
   # basename_LIBRARIES
   # basename_LIBRARY_DEBUG
   # basename_LIBRARY_RELEASE
   # depending on what has been found and set.
   # If only basename_LIBRARY_RELEASE is defined, basename_LIBRARY will be set
   # to the release value, and basename_LIBRARY_DEBUG will be set to 
   # basename_LIBRARY_DEBUG-NOTFOUND. If only basename_LIBRARY_DEBUG is defined, 
   # then basename_LIBRARY will take the debug value, and basename_LIBRARY_RELEASE
   # will be set to basename_LIBRARY_RELEASE-NOTFOUND.
   select_library_configurations(G3LOG)

   if (G3LOG_LIBRARY)
      list(APPEND G3LOG_LIBRARY Threads::Threads)
      if (WIN32)
         list(APPEND G3LOG_LIBRARY DbgHelp.lib)
      endif ()
   endif ()
endif ()

find_package_handle_standard_args(G3LOG REQUIRED_VARS G3LOG_INCLUDE_DIR G3LOG_LIBRARY)
# mark_as_advanced
# Sets the advanced/non-advanced state of the named cached variables.
# An advanced variable will not be displayed in any of the cmake GUIs unless
# the show advanced option is on. In script mode, the advanced/non-advanced
# state has no effect.
mark_as_advanced(G3LOG_INCLUDE_DIR G3LOG_LIBRARY)
set(G3LOG_INCLUDE_DIRS ${G3LOG_INCLUDE_DIR})
set(G3LOG_LIBRARIES ${G3LOG_LIBRARY})
