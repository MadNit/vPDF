# PDFKill — Setup Guide

## Prerequisites

| Tool | Where to get |
|---|---|
| **Qt 6.x** | https://www.qt.io/download (open source) |
| **CMake 3.25+** | https://cmake.org/download |
| **vcpkg** | https://github.com/microsoft/vcpkg |
| **PDFium prebuilts** | https://github.com/bblanchon/pdfium-binaries/releases |

---

## Step 1 — Install QPDF via vcpkg

```bash
vcpkg install qpdf
vcpkg install zlib
```

Make sure to pass `-DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake` to CMake.

---

## Step 2 — Get PDFium prebuilt binaries

1. Download the archive matching your platform from  
   https://github.com/bblanchon/pdfium-binaries/releases

   | Platform | Archive name |
   |---|---|
   | Windows x64 | `pdfium-win-x64.tgz` |
   | macOS arm64 | `pdfium-mac-arm64.tgz` |
   | Linux x64   | `pdfium-linux-x64.tgz` |

2. Extract to `third_party/pdfium/` so the layout is:
   ```
   third_party/pdfium/
     include/   ← fpdfview.h, fpdf_edit.h, fpdf_text.h, ...
     lib/       ← pdfium.lib (Windows) / libpdfium.a (Unix)
     bin/       ← pdfium.dll (Windows only)
   ```

---

## Step 3 — Configure & Build

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_PREFIX_PATH=<Qt-install-root>/lib/cmake

cmake --build build --config Release
```

---

## Project Structure

```
pdfkill/
├── CMakeLists.txt
├── vcpkg.json
├── cmake/
│   └── FindPDFium.cmake       ← locates third_party/pdfium
├── src/
│   ├── main.cpp
│   ├── app/
│   │   ├── MainWindow.h/.cpp  ← top-level window, menus, toolbar
│   ├── engine/
│   │   ├── IPdfEngine.h       ← abstract interface (swap engines here)
│   │   ├── PdfiumEngine.h/.cpp← PDFium rendering & text extraction
│   ├── operations/
│   │   ├── PdfOperations.h/.cpp  ← QPDF-based merge/split/encrypt/etc.
│   ├── ui/
│   │   ├── PageViewer.h/.cpp  ← scrollable page render widget
│   │   └── ThumbnailPanel.h/.cpp ← async thumbnail sidebar
├── resources/
│   └── pdfkill.qrc
└── third_party/
    └── pdfium/                ← place prebuilt PDFium here
```
