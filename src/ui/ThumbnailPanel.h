#pragma once

#include <QListWidget>
#include <QThread>
#include <QObject>
#include <atomic>
#include <memory>

class ThumbnailWorker : public QObject
{
    Q_OBJECT
public:
    explicit ThumbnailWorker(QObject* parent = nullptr);
    void cancelCurrent() { m_cancelled.store(true, std::memory_order_relaxed); }

public slots:
    void processFile(const QString& filePath, int pageCount, float dpi, int generation);

signals:
    void thumbnailReady(int generation, int pageIndex, QImage image);

private:
    std::atomic<bool> m_cancelled{false};
};

class ThumbnailPanel : public QListWidget
{
    Q_OBJECT

public:
    explicit ThumbnailPanel(QWidget* parent = nullptr);
    ~ThumbnailPanel() override;

    void loadFile(const QString& filePath, int pageCount);
    void clear();
    void selectPage(int pageIndex);

signals:
    void pageSelected(int pageIndex);
    void pagesReordered(const QList<int>& orderedPageIndices);
    void rotateLeftRequested(int pageIndex);
    void rotateRightRequested(int pageIndex);
    void deleteRequested(int pageIndex);
    void copyRequested(const QList<int>& pageIndices);
    void cutRequested(const QList<int>& pageIndices);
    void pasteRequested(int insertAfterPageIndex);

protected:
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onThumbnailReady(int generation, int pageIndex, QImage image);
    void onContextMenuRequested(const QPoint& pos);

private:
    void updateDropInsertIndicator(const QPoint& viewportPos);

    QThread* m_thread = nullptr;
    ThumbnailWorker* m_worker = nullptr;
    int m_generation = 0;
    bool m_showDropInsert = false;
    int m_dropInsertRow = -1;

    static constexpr float THUMB_DPI = 20.0f;
    static constexpr int THUMB_W = 90;
    static constexpr int THUMB_H = 120;
};

