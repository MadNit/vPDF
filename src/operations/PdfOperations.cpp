#include "PdfOperations.h"

#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QUuid>
#include <QSet>

// Engine (for extractText and exportToImages)
#include "engine/PdfiumEngine.h"

// QPDF headers
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string toStd(const QString& s) { return s.toStdString(); }

// ---------------------------------------------------------------------------
// Merge
// ---------------------------------------------------------------------------

bool PdfOperations::merge(const QStringList& inputFiles, const QString& outputFile)
{
    try {
        QPDF outPdf;
        outPdf.emptyPDF();
        QPDFPageDocumentHelper outHelper(outPdf);

        for (const QString& path : inputFiles) {
            QPDF src;
            src.processFile(toStd(path).c_str());
            QPDFPageDocumentHelper srcHelper(src);
            for (auto& page : srcHelper.getAllPages())
                outHelper.addPage(outPdf.copyForeignObject(page.getObjectHandle()), false);
        }

        QPDFWriter writer(outPdf, toStd(outputFile).c_str());
        writer.setStaticID(false);
        writer.write();
        return true;
    }
    catch (const std::exception& e) {
        setError(QString::fromStdString(e.what()));
        return false;
    }
}

// ---------------------------------------------------------------------------
// Split
// ---------------------------------------------------------------------------

bool PdfOperations::split(const QString& inputFile, const QString& outputDir)
{
    try {
        QPDF src;
        src.processFile(toStd(inputFile).c_str());
        QPDFPageDocumentHelper srcHelper(src);
        auto pages = srcHelper.getAllPages();

        QDir dir(outputDir);
        const QString stem = QFileInfo(inputFile).baseName();
        const int total = static_cast<int>(pages.size());
        const int digits = QString::number(total).length();

        for (int i = 0; i < total; ++i) {
            QPDF outPdf;
            outPdf.emptyPDF();
            QPDFPageDocumentHelper outHelper(outPdf);
            outHelper.addPage(outPdf.copyForeignObject(pages[i].getObjectHandle()), false);

            const QString outPath = dir.filePath(
                QString("%1_%2.pdf").arg(stem).arg(i + 1, digits, 10, QChar('0')));

            QPDFWriter writer(outPdf, toStd(outPath).c_str());
            writer.write();
        }
        return true;
    }
    catch (const std::exception& e) {
        setError(QString::fromStdString(e.what()));
        return false;
    }
}

// ---------------------------------------------------------------------------
// Compress
// ---------------------------------------------------------------------------

bool PdfOperations::compress(const QString& inputFile, const QString& outputFile)
{
    try {
        QPDF pdf;
        pdf.processFile(toStd(inputFile).c_str());

        QPDFWriter writer(pdf, toStd(outputFile).c_str());
        writer.setCompressStreams(true);
        writer.setRecompressFlate(true);
        writer.setObjectStreamMode(qpdf_o_generate);
        writer.write();
        return true;
    }
    catch (const std::exception& e) {
        setError(QString::fromStdString(e.what()));
        return false;
    }
}

// ---------------------------------------------------------------------------
// Extract text  (delegates rendering to PDFium via the engine)
// ---------------------------------------------------------------------------

bool PdfOperations::extractText(const QString& inputFile, const QString& outputFile)
{
    try {
        PdfiumEngine engine;
        if (!engine.load(inputFile)) {
            setError("Could not load PDF for text extraction.");
            return false;
        }

        QFile out(outputFile);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
            setError("Could not write to output file: " + outputFile);
            return false;
        }

        QTextStream stream(&out);
        for (int i = 0; i < engine.pageCount(); ++i) {
            stream << "=== Page " << (i + 1) << " ===\n";
            stream << engine.extractPageText(i) << "\n\n";
        }
        return true;
    }
    catch (const std::exception& e) {
        setError(QString::fromStdString(e.what()));
        return false;
    }
}

// ---------------------------------------------------------------------------
// Encrypt
// ---------------------------------------------------------------------------

bool PdfOperations::encrypt(const QString& inputFile, const QString& outputFile,
                             const QString& userPassword, const QString& ownerPassword)
{
    try {
        QPDF pdf;
        pdf.processFile(toStd(inputFile).c_str());

        QPDFWriter writer(pdf, toStd(outputFile).c_str());
        writer.setR6EncryptionParameters(
            toStd(userPassword).c_str(),
            toStd(ownerPassword).c_str(),
            /*allow_accessibility=*/       true,
            /*allow_extract=*/             false,
            /*allow_assemble=*/            false,
            /*allow_annotate_and_form=*/   false,
            /*allow_form_filling=*/        false,
            /*allow_modify_other=*/        false,
            /*print=*/                     qpdf_r3p_full,
            /*encrypt_metadata_aes=*/      true);
        writer.write();
        return true;
    }
    catch (const std::exception& e) {
        setError(QString::fromStdString(e.what()));
        return false;
    }
}

// ---------------------------------------------------------------------------
// Decrypt
// ---------------------------------------------------------------------------

bool PdfOperations::decrypt(const QString& inputFile, const QString& outputFile,
                             const QString& password)
{
    try {
        QPDF pdf;
        pdf.processFile(toStd(inputFile).c_str(), toStd(password).c_str());

        QPDFWriter writer(pdf, toStd(outputFile).c_str());
        writer.write();
        return true;
    }
    catch (const std::exception& e) {
        setError(QString::fromStdString(e.what()));
        return false;
    }
}

// ---------------------------------------------------------------------------
// Rotate single page in-place
// ---------------------------------------------------------------------------

bool PdfOperations::rotatePage(const QString& inputFile, int pageIndex, int deltaDegrees)
{
    const QFileInfo fi(inputFile);
    const QString tmpOut = QDir(fi.absolutePath()).filePath(
        QString("%1.rotate-%2.tmp.pdf").arg(fi.completeBaseName(), QUuid::createUuid().toString(QUuid::WithoutBraces)));

    try {
        {
            QPDF pdf;
            pdf.processFile(toStd(inputFile).c_str());

            QPDFPageDocumentHelper dh(pdf);
            auto pages = dh.getAllPages();
            if (pageIndex < 0 || pageIndex >= static_cast<int>(pages.size())) {
                setError("Page index out of range.");
                return false;
            }

            QPDFPageObjectHelper& page = pages[pageIndex];
            QPDFObjectHandle rotObj = page.getObjectHandle().getKey("/Rotate");
            int current = rotObj.isInteger() ? rotObj.getIntValue() : 0;
            int newRot = ((current + deltaDegrees) % 360 + 360) % 360;
            page.getObjectHandle().replaceKey("/Rotate", QPDFObjectHandle::newInteger(newRot));

            QPDFWriter writer(pdf, toStd(tmpOut).c_str());
            writer.write();
        }

        if (QFile::exists(inputFile) && !QFile::remove(inputFile)) {
            QFile::remove(tmpOut);
            setError(QString("Cannot replace original file (remove failed): %1").arg(inputFile));
            return false;
        }
        if (!QFile::rename(tmpOut, inputFile)) {
            QFile::remove(tmpOut);
            setError(QString("Cannot replace original file (rename failed): %1").arg(inputFile));
            return false;
        }

        return true;
    }
    catch (const std::exception& e) {
        QFile::remove(tmpOut);
        setError(QString::fromStdString(e.what()));
        return false;
    }
}


bool PdfOperations::deletePage(const QString& inputFile, int pageIndex)
{
    const QFileInfo fi(inputFile);
    const QString tmpOut = QDir(fi.absolutePath()).filePath(
        QString("%1.delete-%2.tmp.pdf").arg(fi.completeBaseName(), QUuid::createUuid().toString(QUuid::WithoutBraces)));

    try {
        {
            QPDF src;
            src.processFile(toStd(inputFile).c_str());
            QPDFPageDocumentHelper srcHelper(src);
            auto pages = srcHelper.getAllPages();

            const int total = static_cast<int>(pages.size());
            if (pageIndex < 0 || pageIndex >= total) {
                setError("Page index out of range.");
                return false;
            }
            if (total <= 1) {
                setError("Cannot delete the only page in the document.");
                return false;
            }

            QPDF outPdf;
            outPdf.emptyPDF();
            QPDFPageDocumentHelper outHelper(outPdf);

            for (int i = 0; i < total; ++i) {
                if (i == pageIndex) continue;
                outHelper.addPage(outPdf.copyForeignObject(pages[i].getObjectHandle()), false);
            }

            QPDFWriter writer(outPdf, toStd(tmpOut).c_str());
            writer.write();
        }

        if (QFile::exists(inputFile) && !QFile::remove(inputFile)) {
            QFile::remove(tmpOut);
            setError(QString("Cannot replace original file (remove failed): %1").arg(inputFile));
            return false;
        }
        if (!QFile::rename(tmpOut, inputFile)) {
            QFile::remove(tmpOut);
            setError(QString("Cannot replace original file (rename failed): %1").arg(inputFile));
            return false;
        }

        return true;
    }
    catch (const std::exception& e) {
        QFile::remove(tmpOut);
        setError(QString::fromStdString(e.what()));
        return false;
    }
}

bool PdfOperations::reorderPages(const QString& inputFile, const QList<int>& order)
{
    const QFileInfo fi(inputFile);
    const QString tmpOut = QDir(fi.absolutePath()).filePath(
        QString("%1.reorder-%2.tmp.pdf").arg(fi.completeBaseName(), QUuid::createUuid().toString(QUuid::WithoutBraces)));

    try {
        {
            QPDF src;
            src.processFile(toStd(inputFile).c_str());
            QPDFPageDocumentHelper srcHelper(src);
            auto pages = srcHelper.getAllPages();
            const int total = static_cast<int>(pages.size());

            if (order.size() != total) {
                setError("Invalid reorder vector size.");
                return false;
            }

            QSet<int> seen;
            for (int idx : order) {
                if (idx < 0 || idx >= total || seen.contains(idx)) {
                    setError("Invalid reorder vector entries.");
                    return false;
                }
                seen.insert(idx);
            }

            QPDF outPdf;
            outPdf.emptyPDF();
            QPDFPageDocumentHelper outHelper(outPdf);

            for (int idx : order) {
                outHelper.addPage(outPdf.copyForeignObject(pages[idx].getObjectHandle()), false);
            }

            QPDFWriter writer(outPdf, toStd(tmpOut).c_str());
            writer.write();
        }

        if (QFile::exists(inputFile) && !QFile::remove(inputFile)) {
            QFile::remove(tmpOut);
            setError(QString("Cannot replace original file (remove failed): %1").arg(inputFile));
            return false;
        }
        if (!QFile::rename(tmpOut, inputFile)) {
            QFile::remove(tmpOut);
            setError(QString("Cannot replace original file (rename failed): %1").arg(inputFile));
            return false;
        }

        return true;
    }
    catch (const std::exception& e) {
        QFile::remove(tmpOut);
        setError(QString::fromStdString(e.what()));
        return false;
    }
}


bool PdfOperations::deletePages(const QString& inputFile, const QList<int>& pageIndices)
{
    const QFileInfo fi(inputFile);
    const QString tmpOut = QDir(fi.absolutePath()).filePath(
        QString("%1.delete-many-%2.tmp.pdf").arg(fi.completeBaseName(), QUuid::createUuid().toString(QUuid::WithoutBraces)));

    try {
        {
            QPDF src;
            src.processFile(toStd(inputFile).c_str());
            QPDFPageDocumentHelper srcHelper(src);
            auto pages = srcHelper.getAllPages();
            const int total = static_cast<int>(pages.size());

            QSet<int> drop;
            for (int idx : pageIndices) {
                if (idx < 0 || idx >= total) {
                    setError("Page index out of range.");
                    return false;
                }
                drop.insert(idx);
            }
            if (drop.isEmpty()) {
                setError("No pages selected.");
                return false;
            }
            if (drop.size() >= total) {
                setError("Cannot delete all pages from document.");
                return false;
            }

            QPDF outPdf;
            outPdf.emptyPDF();
            QPDFPageDocumentHelper outHelper(outPdf);
            for (int i = 0; i < total; ++i) {
                if (drop.contains(i)) continue;
                outHelper.addPage(outPdf.copyForeignObject(pages[i].getObjectHandle()), false);
            }

            QPDFWriter writer(outPdf, toStd(tmpOut).c_str());
            writer.write();
        }

        if (QFile::exists(inputFile) && !QFile::remove(inputFile)) {
            QFile::remove(tmpOut);
            setError(QString("Cannot replace original file (remove failed): %1").arg(inputFile));
            return false;
        }
        if (!QFile::rename(tmpOut, inputFile)) {
            QFile::remove(tmpOut);
            setError(QString("Cannot replace original file (rename failed): %1").arg(inputFile));
            return false;
        }

        return true;
    }
    catch (const std::exception& e) {
        QFile::remove(tmpOut);
        setError(QString::fromStdString(e.what()));
        return false;
    }
}

bool PdfOperations::extractPages(const QString& inputFile, const QList<int>& pageIndices, const QString& outputFile)
{
    try {
        QPDF src;
        src.processFile(toStd(inputFile).c_str());
        QPDFPageDocumentHelper srcHelper(src);
        auto pages = srcHelper.getAllPages();
        const int total = static_cast<int>(pages.size());

        if (pageIndices.isEmpty()) {
            setError("No pages selected.");
            return false;
        }

        QPDF outPdf;
        outPdf.emptyPDF();
        QPDFPageDocumentHelper outHelper(outPdf);

        for (int idx : pageIndices) {
            if (idx < 0 || idx >= total) {
                setError("Page index out of range.");
                return false;
            }
            outHelper.addPage(outPdf.copyForeignObject(pages[idx].getObjectHandle()), false);
        }

        QPDFWriter writer(outPdf, toStd(outputFile).c_str());
        writer.write();
        return true;
    }
    catch (const std::exception& e) {
        setError(QString::fromStdString(e.what()));
        return false;
    }
}

bool PdfOperations::insertPages(const QString& targetFile, const QString& pagesFile, int insertIndex)
{
    const QFileInfo fi(targetFile);
    const QString tmpOut = QDir(fi.absolutePath()).filePath(
        QString("%1.insert-%2.tmp.pdf").arg(fi.completeBaseName(), QUuid::createUuid().toString(QUuid::WithoutBraces)));

    try {
        {
            QPDF target;
            target.processFile(toStd(targetFile).c_str());
            QPDFPageDocumentHelper targetHelper(target);
            auto targetPages = targetHelper.getAllPages();

            QPDF src;
            src.processFile(toStd(pagesFile).c_str());
            QPDFPageDocumentHelper srcHelper(src);
            auto srcPages = srcHelper.getAllPages();

            const int targetTotal = static_cast<int>(targetPages.size());
            if (insertIndex < 0 || insertIndex > targetTotal) {
                setError("Insert index out of range.");
                return false;
            }

            QPDF outPdf;
            outPdf.emptyPDF();
            QPDFPageDocumentHelper outHelper(outPdf);

            for (int i = 0; i <= targetTotal; ++i) {
                if (i == insertIndex) {
                    for (const auto& sp : srcPages) {
                        outHelper.addPage(outPdf.copyForeignObject(sp.getObjectHandle()), false);
                    }
                }
                if (i < targetTotal) {
                    outHelper.addPage(outPdf.copyForeignObject(targetPages[i].getObjectHandle()), false);
                }
            }

            QPDFWriter writer(outPdf, toStd(tmpOut).c_str());
            writer.write();
        }

        if (QFile::exists(targetFile) && !QFile::remove(targetFile)) {
            QFile::remove(tmpOut);
            setError(QString("Cannot replace original file (remove failed): %1").arg(targetFile));
            return false;
        }
        if (!QFile::rename(tmpOut, targetFile)) {
            QFile::remove(tmpOut);
            setError(QString("Cannot replace original file (rename failed): %1").arg(targetFile));
            return false;
        }

        return true;
    }
    catch (const std::exception& e) {
        QFile::remove(tmpOut);
        setError(QString::fromStdString(e.what()));
        return false;
    }
}

// ---------------------------------------------------------------------------
// Export pages to images
// ---------------------------------------------------------------------------

bool PdfOperations::exportToImages(const QString& inputFile, const QString& outputDir,
                                    IPdfEngine* engine, float dpi)
{
    if (!engine || !engine->isOpen()) {
        setError("No PDF document is open in the engine.");
        return false;
    }

    QDir dir(outputDir);
    const QString stem = QFileInfo(inputFile).baseName();
    const int total  = engine->pageCount();
    const int digits = QString::number(total).length();

    for (int i = 0; i < total; ++i) {
        QImage img = engine->renderPage(i, dpi);
        if (img.isNull()) {
            setError(QString("Render failed on page %1.").arg(i + 1));
            return false;
        }
        const QString outPath = dir.filePath(
            QString("%1_%2.png").arg(stem).arg(i + 1, digits, 10, QChar('0')));
        if (!img.save(outPath, "PNG")) {
            setError(QString("Could not save image: %1").arg(outPath));
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Import images → PDF
// ---------------------------------------------------------------------------

bool PdfOperations::importFromImages(const QStringList& imageFiles,
                                      const QString& outputFile)
{
    try {
        QPDF pdf;
        pdf.emptyPDF();
        QPDFPageDocumentHelper dh(pdf);

        QPDFObjectHandle pagesObj = pdf.getRoot().getKey("/Pages");

        for (const QString& imgPath : imageFiles) {
            QImage img(imgPath);
            if (img.isNull()) {
                setError(QString("Could not load image: %1").arg(imgPath));
                return false;
            }

            // Convert to RGB888 for JPEG-like embedding via raw stream
            img = img.convertToFormat(QImage::Format_RGB888);

            const double ptW = img.width()  * 72.0 / qMax(img.dotsPerMeterX() * 0.0254, 72.0);
            const double ptH = img.height() * 72.0 / qMax(img.dotsPerMeterY() * 0.0254, 72.0);

            // Build image XObject dictionary
            QPDFObjectHandle imgDict = QPDFObjectHandle::newDictionary();
            imgDict.replaceKey("/Type",             QPDFObjectHandle::newName("/XObject"));
            imgDict.replaceKey("/Subtype",          QPDFObjectHandle::newName("/Image"));
            imgDict.replaceKey("/Width",            QPDFObjectHandle::newInteger(img.width()));
            imgDict.replaceKey("/Height",           QPDFObjectHandle::newInteger(img.height()));
            imgDict.replaceKey("/ColorSpace",       QPDFObjectHandle::newName("/DeviceRGB"));
            imgDict.replaceKey("/BitsPerComponent", QPDFObjectHandle::newInteger(8));

            QByteArray rawBytes(reinterpret_cast<const char*>(img.constBits()),
                                img.sizeInBytes());
            QPDFObjectHandle imageStream =
                QPDFObjectHandle::newStream(&pdf, rawBytes.toStdString());
            imageStream.replaceDict(imgDict);

            QPDFObjectHandle imageRef = pdf.makeIndirectObject(imageStream);

            // Content stream: draw the image filling the page
            std::string cs =
                "q\n" +
                std::to_string(ptW) + " 0 0 " + std::to_string(ptH) + " 0 0 cm\n" +
                "/Im0 Do\n"
                "Q\n";

            QPDFObjectHandle contentStream =
                QPDFObjectHandle::newStream(&pdf, cs);

            // Resources dict
            QPDFObjectHandle xobjects = QPDFObjectHandle::newDictionary();
            xobjects.replaceKey("/Im0", imageRef);
            QPDFObjectHandle resources = QPDFObjectHandle::newDictionary();
            resources.replaceKey("/XObject", xobjects);

            // Page dict
            QPDFObjectHandle pageDict = QPDFObjectHandle::newDictionary();
            pageDict.replaceKey("/Type",      QPDFObjectHandle::newName("/Page"));
            pageDict.replaceKey("/Parent",    pagesObj);
            pageDict.replaceKey("/Resources", resources);
            pageDict.replaceKey("/Contents",  contentStream);

            QPDFObjectHandle mediaBox = QPDFObjectHandle::newArray();
            mediaBox.appendItem(QPDFObjectHandle::newInteger(0));
            mediaBox.appendItem(QPDFObjectHandle::newInteger(0));
            mediaBox.appendItem(QPDFObjectHandle::newReal(ptW));
            mediaBox.appendItem(QPDFObjectHandle::newReal(ptH));
            pageDict.replaceKey("/MediaBox", mediaBox);

            QPDFObjectHandle pageRef = pdf.makeIndirectObject(pageDict);
            dh.addPage(pageRef, false);
        }

        QPDFWriter writer(pdf, toStd(outputFile).c_str());
        writer.write();
        return true;
    }
    catch (const std::exception& e) {
        setError(QString::fromStdString(e.what()));
        return false;
    }
}






