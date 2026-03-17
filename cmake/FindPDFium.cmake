# FindPDFium.cmake
#
# Locates prebuilt PDFium binaries from:
#   https://github.com/bblanchon/pdfium-binaries
#
# Expected directory layout after extracting the archive:
#   third_party/pdfium/
#     include/       <- fpdfview.h, fpdf_edit.h, ...
#     lib/           <- pdfium.lib (Windows) / libpdfium.a (Unix)
#     bin/           <- pdfium.dll (Windows only)
#
# Sets:
#   PDFIUM_FOUND
#   PDFIUM_INCLUDE_DIRS
#   PDFIUM_LIBRARIES
#   PDFIUM_RUNTIME_LIBRARY   (Windows: full path to pdfium.dll)
#
# Provides imported target: PDFium::PDFium

set(_PDFIUM_ROOT "${CMAKE_SOURCE_DIR}/third_party/pdfium"
    CACHE PATH "Root directory of prebuilt PDFium binaries")

find_path(PDFIUM_INCLUDE_DIRS
    NAMES fpdfview.h
    PATHS "${_PDFIUM_ROOT}/include"
    NO_DEFAULT_PATH
)

if(WIN32)
    find_library(PDFIUM_LIBRARIES
        NAMES pdfium
        PATHS "${_PDFIUM_ROOT}/lib"
        NO_DEFAULT_PATH
    )
    find_file(PDFIUM_RUNTIME_LIBRARY
        NAMES pdfium.dll
        PATHS "${_PDFIUM_ROOT}/bin"
        NO_DEFAULT_PATH
    )
elseif(APPLE)
    find_library(PDFIUM_LIBRARIES
        NAMES pdfium
        PATHS "${_PDFIUM_ROOT}/lib"
        NO_DEFAULT_PATH
    )
else()  # Linux
    find_library(PDFIUM_LIBRARIES
        NAMES pdfium
        PATHS "${_PDFIUM_ROOT}/lib"
        NO_DEFAULT_PATH
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PDFium
    REQUIRED_VARS PDFIUM_LIBRARIES PDFIUM_INCLUDE_DIRS
)

if(PDFium_FOUND AND NOT TARGET PDFium::PDFium)
    if(WIN32)
        # Windows: SHARED with a separate import lib (.lib) and runtime DLL
        add_library(PDFium::PDFium SHARED IMPORTED)
        set_target_properties(PDFium::PDFium PROPERTIES
            IMPORTED_IMPLIB   "${PDFIUM_LIBRARIES}"
            IMPORTED_LOCATION "${PDFIUM_RUNTIME_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${PDFIUM_INCLUDE_DIRS}"
        )
    else()
        # Linux / macOS: static or shared .a / .so / .dylib — no import lib
        add_library(PDFium::PDFium UNKNOWN IMPORTED)
        set_target_properties(PDFium::PDFium PROPERTIES
            IMPORTED_LOCATION "${PDFIUM_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${PDFIUM_INCLUDE_DIRS}"
        )
    endif()
endif()

mark_as_advanced(PDFIUM_INCLUDE_DIRS PDFIUM_LIBRARIES PDFIUM_RUNTIME_LIBRARY)
