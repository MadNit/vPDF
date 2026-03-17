#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "qpdf::libqpdf" for configuration "Debug"
set_property(TARGET qpdf::libqpdf APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(qpdf::libqpdf PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/qpdf.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/qpdf30.dll"
  )

list(APPEND _cmake_import_check_targets qpdf::libqpdf )
list(APPEND _cmake_import_check_files_for_qpdf::libqpdf "${_IMPORT_PREFIX}/debug/lib/qpdf.lib" "${_IMPORT_PREFIX}/debug/bin/qpdf30.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
