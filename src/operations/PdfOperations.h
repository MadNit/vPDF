#pragma once

#include <QString>
#include <QStringList>
#include <QList>

#include "engine/IPdfEngine.h"

class PdfOperations
{
public:
    PdfOperations() = default;

    bool merge(const QStringList& inputFiles, const QString& outputFile);
    bool split(const QString& inputFile, const QString& outputDir);
    bool compress(const QString& inputFile, const QString& outputFile);
    bool extractText(const QString& inputFile, const QString& outputFile);
    bool encrypt(const QString& inputFile, const QString& outputFile,
                 const QString& userPassword, const QString& ownerPassword);
    bool decrypt(const QString& inputFile, const QString& outputFile,
                 const QString& password);

    bool rotatePage(const QString& inputFile, int pageIndex, int deltaDegrees);
    bool deletePage(const QString& inputFile, int pageIndex);
    bool deletePages(const QString& inputFile, const QList<int>& pageIndices);
    bool reorderPages(const QString& inputFile, const QList<int>& order);
    bool extractPages(const QString& inputFile, const QList<int>& pageIndices, const QString& outputFile);
    bool insertPages(const QString& targetFile, const QString& pagesFile, int insertIndex);

    bool exportToImages(const QString& inputFile, const QString& outputDir,
                        IPdfEngine* engine, float dpi = 150.0f);
    bool importFromImages(const QStringList& imageFiles, const QString& outputFile);

    QString lastError() const { return m_lastError; }

private:
    QString m_lastError;
    void setError(const QString& msg) { m_lastError = msg; }
};

