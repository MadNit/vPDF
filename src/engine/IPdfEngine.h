#pragma once

#include <QString>
#include <QImage>
#include <QSizeF>
#include <QRectF>
#include <QVector>

// ---------------------------------------------------------------------------
// IPdfEngine
// Abstract interface for PDF rendering/parsing.
// Swap implementations without touching the rest of the app.
// ---------------------------------------------------------------------------
class IPdfEngine
{
public:
    virtual ~IPdfEngine() = default;

    // --- Document lifecycle ---
    virtual bool load(const QString& filePath, const QString& password = {}) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    // --- Document metadata ---
    virtual int     pageCount()  const = 0;
    virtual QString title()      const = 0;
    virtual QString author()     const = 0;
    virtual QString subject()    const = 0;
    virtual QString keywords()   const = 0;
    virtual QString creator()    const = 0;
    virtual QString producer()   const = 0;

    // --- Page info ---
    virtual QSizeF  pageSize(int pageIndex)   const = 0;  // in PDF points (1/72 inch)

    // --- Rendering ---
    // Renders a page at the given DPI to a QImage (RGBA).
    virtual QImage  renderPage(int pageIndex, float dpi = 150.0f) const = 0;

    // --- Text extraction ---
    virtual QString extractPageText(int pageIndex) const = 0;
    // Returns match boxes in PDF point coordinates (origin: bottom-left).
    virtual QVector<QRectF> searchPageTextRects(int pageIndex, const QString& term) const = 0;
};
