#pragma once

#include <atomic>
#include "IPdfEngine.h"

// Forward-declare PDFium types to keep the header clean.
struct fpdf_document_t__;
using FPDF_DOCUMENT = fpdf_document_t__*;

// ---------------------------------------------------------------------------
// PdfiumEngine
// Concrete IPdfEngine backed by Google's PDFium library.
// ---------------------------------------------------------------------------
class PdfiumEngine : public IPdfEngine
{
public:
    PdfiumEngine();
    ~PdfiumEngine() override;

    // IPdfEngine interface
    bool    load(const QString& filePath, const QString& password = {}) override;
    void    close()     override;
    bool    isOpen()    const override;

    int     pageCount() const override;
    QString title()     const override;
    QString author()    const override;
    QString subject()   const override;
    QString keywords()  const override;
    QString creator()   const override;
    QString producer()  const override;

    QSizeF  pageSize(int pageIndex)               const override;
    QImage  renderPage(int pageIndex, float dpi)  const override;
    QString extractPageText(int pageIndex)         const override;
    QVector<QRectF> searchPageTextRects(int pageIndex, const QString& term) const override;

private:
    QString getMetaText(const char* tag) const;

    FPDF_DOCUMENT m_doc   = nullptr;
    bool          m_open  = false;

    static std::atomic<int> s_instanceCount;  // ref-count PDFium global init (accessed from multiple threads)
};
