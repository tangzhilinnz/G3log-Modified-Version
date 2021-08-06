#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "g3log" for configuration "Debug"
set_property(TARGET g3log APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(g3log PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/g3log.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/g3log.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS g3log )
list(APPEND _IMPORT_CHECK_FILES_FOR_g3log "${_IMPORT_PREFIX}/lib/g3log.lib" "${_IMPORT_PREFIX}/bin/g3log.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
