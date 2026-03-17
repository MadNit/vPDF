cmake_minimum_required(VERSION 3.25)
project(pdfkill VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ---------------------------------------------------------------
# Qt6
# ---------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# ---------------------------------------------------------------
# PDFium  (uses bundled PDFiumConfig.cmake from the prebuilt archive)
# Download from: https://github.com/bblanchon/pdfium-binaries
# Extract to: third_party/pdfium/
# ---------------------------------------------------------------
set(PDFium_DIR "${CMAKE_SOURCE_DIR}/third_party/pdfium" CACHE PATH "PDFium install dir")
find_package(PDFium REQUIRED)

# ---------------------------------------------------------------
# QPDF  (install via vcpkg: vcpkg install qpdf)
# ---------------------------------------------------------------
find_package(qpdf CONFIG REQUIRED)

# ---------------------------------------------------------------
# Sources
# ---------------------------------------------------------------
set(SOURCES
    src/main.cpp
    src/app/MainWindow.cpp
    src/engine/PdfiumEngine.cpp
    src/operations/PdfOperations.cpp
    src/ui/PageViewer.cpp
    src/ui/ThumbnailPanel.cpp
    src/ui/SidebarMenu.cpp
    src/ui/CollapsiblePanel.cpp
)

set(HEADERS
    src/app/MainWindow.h
    src/engine/IPdfEngine.h
    src/engine/PdfiumEngine.h
    src/operations/PdfOperations.h
    src/ui/PageViewer.h
    src/ui/ThumbnailPanel.h
    src/ui/SidebarMenu.h
    src/ui/CollapsiblePanel.h
)

# ---------------------------------------------------------------
# Target
# ---------------------------------------------------------------
add_executable(pdfkill WIN32 MACOSX_BUNDLE
    ${SOURCES}
    ${HEADERS}
    resources/pdfkill.qrc
)

target_include_directories(pdfkill PRIVATE
    src
)

target_link_libraries(pdfkill PRIVATE
    Qt6::Widgets
    pdfium
    qpdf::libqpdf
)

# ---------------------------------------------------------------
# Install
# ---------------------------------------------------------------
install(TARGETS pdfkill DESTINATION bin)
if(WIN32)
    install(FILES "${CMAKE_SOURCE_DIR}/third_party/pdfium/bin/pdfium.dll" DESTINATION bin)
endif()
