#include "PdfiumEngine.h"

#include <QDebug>
#include <QFile>
#include <mutex>

// PDFium public API headers
#include <fpdfview.h>
#include <fpdf_text.h>
#include <fpdf_doc.h>

namespace {
std::mutex g_pdfiumMutex;
}

// ---------------------------------------------------------------------------
// Static init / teardown
// ---------------------------------------------------------------------------

std::atomic<int> PdfiumEngine::s_instanceCount = 0;

PdfiumEngine::PdfiumEngine()
{
    std::lock_guard<std::mutex> lock(g_pdfiumMutex);
    const int prev = s_instanceCount.fetch_add(1, std::memory_order_acq_rel);
    if (prev == 0) {
        FPDF_InitLibrary();
    }
}

PdfiumEngine::~PdfiumEngine()
{
    close();

    std::lock_guard<std::mutex> lock(g_pdfiumMutex);
    const int prev = s_instanceCount.fetch_sub(1, std::memory_order_acq_rel);
    if (prev == 1) {
        FPDF_DestroyLibrary();
    }
}

// ---------------------------------------------------------------------------
// Document lifecycle
// ---------------------------------------------------------------------------

bool PdfiumEngine::load(const QString& filePath, const QString& password)
{
    close();

    std::lock_guard<std::mutex> lock(g_pdfiumMutex);
    const QByteArray path8 = filePath.toUtf8();
    const QByteArray pass8 = password.toUtf8();

    m_doc = FPDF_LoadDocument(
        path8.constData(),
        password.isEmpty() ? nullptr : pass8.constData());

    if (!m_doc) {
        unsigned long err = FPDF_GetLastError();
        qWarning() << "PDFium: failed to load" << filePath << "error:" << err;
        return false;
    }

    m_open = true;
    return true;
}

void PdfiumEngine::close()
{
    std::lock_guard<std::mutex> lock(g_pdfiumMutex);
    if (m_doc) {
        FPDF_CloseDocument(m_doc);
        m_doc = nullptr;
    }
    m_open = false;
}

bool PdfiumEngine::isOpen() const
{
    return m_open;
}

// ---------------------------------------------------------------------------
// Metadata
// ---------------------------------------------------------------------------

int PdfiumEngine::pageCount() const
{
    std::lock_guard<std::mutex> lock(g_pdfiumMutex);
    if (!m_doc) return 0;
    return FPDF_GetPageCount(m_doc);
}

QString PdfiumEngine::getMetaText(const char* tag) const
{
    std::lock_guard<std::mutex> lock(g_pdfiumMutex);
    if (!m_doc) return {};

    // First call to get required buffer size
    unsigned long len = FPDF_GetMetaText(m_doc, tag, nullptr, 0);
    if (len == 0) return {};

    // PDFium returns UTF-16LE
    QByteArray buf(static_cast<int>(len), '\0');
    FPDF_GetMetaText(m_doc, tag, buf.data(), static_cast<unsigned long>(buf.size()));

    return QString::fromUtf16(
        reinterpret_cast<const char16_t*>(buf.constData()),
        static_cast<qsizetype>(len / 2 - 1));  // -1 to strip null terminator
}

QString PdfiumEngine::title()    const { return getMetaText("Title");    }
QString PdfiumEngine::author()   const { return getMetaText("Author");   }
QString PdfiumEngine::subject()  const { return getMetaText("Subject");  }
QString PdfiumEngine::keywords() const { return getMetaText("Keywords"); }
QString PdfiumEngine::creator()  const { return getMetaText("Creator");  }
QString PdfiumEngine::producer() const { return getMetaText("Producer"); }

// ---------------------------------------------------------------------------
// Page info
// ---------------------------------------------------------------------------

QSizeF PdfiumEngine::pageSize(int pageIndex) const
{
    std::lock_guard<std::mutex> lock(g_pdfiumMutex);
    if (!m_doc) return {};

    double w = 0.0, h = 0.0;
    FPDF_GetPageSizeByIndex(m_doc, pageIndex, &w, &h);
    return { w, h };
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

QImage PdfiumEngine::renderPage(int pageIndex, float dpi) const
{
    std::lock_guard<std::mutex> lock(g_pdfiumMutex);
    if (!m_doc) return {};

    FPDF_PAGE page = FPDF_LoadPage(m_doc, pageIndex);
    if (!page) return {};

    const double pageW = FPDF_GetPageWidth(page);
    const double pageH = FPDF_GetPageHeight(page);

    // Convert PDF points (1/72") to pixels at the requested DPI
    const int pixW = static_cast<int>(pageW * dpi / 72.0);
    const int pixH = static_cast<int>(pageH * dpi / 72.0);

    QImage image(pixW, pixH, QImage::Format_ARGB32);
    image.fill(Qt::white);

    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(
        pixW, pixH,
        FPDFBitmap_BGRA,
        image.bits(),
        image.bytesPerLine());

    if (bitmap) {
        FPDF_RenderPageBitmap(bitmap, page,
            0, 0, pixW, pixH,
            0,                          // rotation: 0 = normal
            FPDF_ANNOT | FPDF_LCD_TEXT);
        FPDFBitmap_Destroy(bitmap);
    }

    FPDF_ClosePage(page);

    // PDFium writes BGRA; Qt ARGB32 on little-endian is stored BGRA in memory.
    // On big-endian machines an RGBA swap would be needed - handled below.
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    image = image.rgbSwapped();
#endif
    return image;
}

// ---------------------------------------------------------------------------
// Text extraction
// ---------------------------------------------------------------------------

QString PdfiumEngine::extractPageText(int pageIndex) const
{
    std::lock_guard<std::mutex> lock(g_pdfiumMutex);
    if (!m_doc) return {};

    FPDF_PAGE page = FPDF_LoadPage(m_doc, pageIndex);
    if (!page) return {};

    FPDF_TEXTPAGE textPage = FPDFText_LoadPage(page);
    if (!textPage) {
        FPDF_ClosePage(page);
        return {};
    }

    int charCount = FPDFText_CountChars(textPage);
    if (charCount <= 0) {
        FPDFText_ClosePage(textPage);
        FPDF_ClosePage(page);
        return {};
    }

    // GetText writes UTF-16LE, needs charCount+1 chars for null terminator
    QVector<unsigned short> buf(charCount + 1, 0);
    FPDFText_GetText(textPage, 0, charCount,
                     reinterpret_cast<unsigned short*>(buf.data()));

    FPDFText_ClosePage(textPage);
    FPDF_ClosePage(page);

    return QString::fromUtf16(
        reinterpret_cast<const char16_t*>(buf.constData()), charCount);
}

QVector<QRectF> PdfiumEngine::searchPageTextRects(int pageIndex, const QString& term) const
{
    std::lock_guard<std::mutex> lock(g_pdfiumMutex);
    QVector<QRectF> rects;

    if (!m_doc || term.trimmed().isEmpty()) {
        return rects;
    }

    FPDF_PAGE page = FPDF_LoadPage(m_doc, pageIndex);
    if (!page) {
        return rects;
    }

    FPDF_TEXTPAGE textPage = FPDFText_LoadPage(page);
    if (!textPage) {
        FPDF_ClosePage(page);
        return rects;
    }

    const QString needle = term.trimmed();
    QVector<unsigned short> pattern(needle.size() + 1, 0);
    for (int i = 0; i < needle.size(); ++i) {
        pattern[i] = static_cast<unsigned short>(needle.at(i).unicode());
    }

    FPDF_SCHHANDLE finder = FPDFText_FindStart(textPage, pattern.data(), 0, 0);
    if (finder) {
        while (FPDFText_FindNext(finder)) {
            const int start = FPDFText_GetSchResultIndex(finder);
            const int count = FPDFText_GetSchCount(finder);

            for (int i = 0; i < count; ++i) {
                double left = 0.0;
                double right = 0.0;
                double bottom = 0.0;
                double top = 0.0;
                const bool ok = FPDFText_GetCharBox(textPage, start + i, &left, &right, &bottom, &top);
                if (!ok) {
                    continue;
                }
                rects.push_back(QRectF(left, bottom, qMax(0.0, right - left), qMax(0.0, top - bottom)));
            }
        }
        FPDFText_FindClose(finder);
    }

    FPDFText_ClosePage(textPage);
    FPDF_ClosePage(page);

    if (rects.size() <= 1) {
        return rects;
    }

    QVector<QRectF> merged;
    merged.reserve(rects.size());
    for (const QRectF& r : rects) {
        if (merged.isEmpty()) {
            merged.push_back(r);
            continue;
        }

        QRectF& last = merged.last();
        const bool sameLine = qAbs(last.y() - r.y()) < 1.5;
        const bool adjacent = qAbs((last.x() + last.width()) - r.x()) < 2.0;
        if (sameLine && adjacent) {
            last = last.united(r);
        } else {
            merged.push_back(r);
        }
    }

    return merged;
}
