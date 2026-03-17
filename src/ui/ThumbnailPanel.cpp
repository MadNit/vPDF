#include "ThumbnailPanel.h"

#include <QListWidgetItem>
#include <QIcon>
#include <QPixmap>
#include <QMetaObject>
#include <QMenu>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QKeyEvent>
#include <QKeySequence>

#include "engine/PdfiumEngine.h"

ThumbnailWorker::ThumbnailWorker(QObject* parent)
    : QObject(parent)
{}

void ThumbnailWorker::processFile(const QString& filePath, int pageCount, float dpi, int generation)
{
    m_cancelled.store(false, std::memory_order_relaxed);

    auto engine = std::make_unique<PdfiumEngine>();
    if (!engine->load(filePath)) return;

    for (int i = 0; i < pageCount; ++i) {
        if (m_cancelled.load(std::memory_order_relaxed)) break;
        QImage img = engine->renderPage(i, dpi);
        if (m_cancelled.load(std::memory_order_relaxed)) break;
        if (!img.isNull()) {
            emit thumbnailReady(generation, i, img);
        }
    }
}

ThumbnailPanel::ThumbnailPanel(QWidget* parent)
    : QListWidget(parent)
{
    setViewMode(QListView::IconMode);
    setFlow(QListView::TopToBottom);
    setWrapping(false);
    setIconSize(QSize(THUMB_W, THUMB_H));
    setUniformItemSizes(true);
    setSpacing(6);
    setMovement(QListView::Snap);
    setResizeMode(QListView::Fixed);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    setStyleSheet(
        "QListWidget { background: #EFEFEF; border: none; }"
        "QListWidget::item { background: #FFFFFF; border: 1px solid #C9C9C9; margin: 4px 8px; padding: 6px; }"
        "QListWidget::item:selected { background: #F6E7B4; border: 2px solid #E2B53A; }"
        "QListWidget::item:hover { border: 1px solid #AFAFAF; }");

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropOverwriteMode(false);
    setAutoScroll(true);
    setAutoScrollMargin(28);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QListWidget::itemClicked, this, &ThumbnailPanel::onItemClicked);
    connect(this, &QWidget::customContextMenuRequested, this, &ThumbnailPanel::onContextMenuRequested);

    m_thread = new QThread(this);
    m_worker = new ThumbnailWorker();
    m_worker->moveToThread(m_thread);

    connect(m_worker, &ThumbnailWorker::thumbnailReady,
            this, &ThumbnailPanel::onThumbnailReady,
            Qt::QueuedConnection);
    connect(m_thread, &QThread::finished,
            m_worker, &QObject::deleteLater);

    m_thread->start(QThread::LowPriority);
}

ThumbnailPanel::~ThumbnailPanel()
{
    m_worker->cancelCurrent();
    m_thread->quit();
    m_thread->wait();
}

void ThumbnailPanel::loadFile(const QString& filePath, int pageCount)
{
    m_worker->cancelCurrent();
    ++m_generation;

    QListWidget::clear();
    if (filePath.isEmpty() || pageCount <= 0) return;

    for (int i = 0; i < pageCount; ++i) {
        auto* it = new QListWidgetItem(tr("Page %1").arg(i + 1), this);
        it->setTextAlignment(Qt::AlignCenter);
        it->setData(Qt::UserRole, i);
        it->setSizeHint(QSize(THUMB_W + 14, THUMB_H + 38));
    }

    QMetaObject::invokeMethod(m_worker, "processFile",
                              Qt::QueuedConnection,
                              Q_ARG(QString, filePath),
                              Q_ARG(int, pageCount),
                              Q_ARG(float, THUMB_DPI),
                              Q_ARG(int, m_generation));
}

void ThumbnailPanel::clear()
{
    m_worker->cancelCurrent();
    m_showDropInsert = false;
    m_dropInsertRow = -1;
    QListWidget::clear();
}

void ThumbnailPanel::selectPage(int pageIndex)
{
    if (pageIndex < 0 || pageIndex >= count()) return;
    setCurrentRow(pageIndex);
    scrollToItem(item(pageIndex), QAbstractItemView::PositionAtCenter);
}

void ThumbnailPanel::updateDropInsertIndicator(const QPoint& viewportPos)
{
    int insertRow = 0;
    if (count() <= 0) {
        insertRow = 0;
    } else {
        QListWidgetItem* target = itemAt(viewportPos);
        if (!target) {
            const QRect firstRect = visualItemRect(item(0));
            const QRect lastRect = visualItemRect(item(count() - 1));
            if (viewportPos.y() <= firstRect.center().y()) {
                insertRow = 0;
            } else if (viewportPos.y() >= lastRect.center().y()) {
                insertRow = count();
            } else {
                insertRow = count();
            }
        } else {
            const int rowAt = row(target);
            const QRect r = visualItemRect(target);
            insertRow = (viewportPos.y() <= r.center().y()) ? rowAt : (rowAt + 1);
        }
    }

    if (!m_showDropInsert || m_dropInsertRow != insertRow) {
        m_showDropInsert = true;
        m_dropInsertRow = insertRow;
        viewport()->update();
    }
}

void ThumbnailPanel::dragMoveEvent(QDragMoveEvent* event)
{
    QListWidget::dragMoveEvent(event);

    const int margin = 28;
    const int step = 16;
    if (auto* sb = verticalScrollBar()) {
        if (event->position().y() < margin) {
            sb->setValue(sb->value() - step);
        } else if (event->position().y() > (viewport()->height() - margin)) {
            sb->setValue(sb->value() + step);
        }
    }

    updateDropInsertIndicator(event->position().toPoint());
    event->acceptProposedAction();
}

void ThumbnailPanel::dragLeaveEvent(QDragLeaveEvent* event)
{
    QListWidget::dragLeaveEvent(event);
    m_showDropInsert = false;
    m_dropInsertRow = -1;
    viewport()->update();
}

void ThumbnailPanel::dropEvent(QDropEvent* event)
{
    m_showDropInsert = false;
    m_dropInsertRow = -1;

    QListWidget::dropEvent(event);

    QList<int> order;
    order.reserve(count());
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem* it = item(i);
        if (!it) continue;
        order.append(it->data(Qt::UserRole).toInt());
        it->setText(tr("Page %1").arg(i + 1));
    }

    viewport()->update();

    if (!order.isEmpty()) {
        emit pagesReordered(order);
    }
}

void ThumbnailPanel::paintEvent(QPaintEvent* event)
{
    QListWidget::paintEvent(event);

    if (!m_showDropInsert || m_dropInsertRow < 0) {
        return;
    }

    int y = 0;
    if (count() <= 0) {
        y = 6;
    } else if (m_dropInsertRow <= 0) {
        y = visualItemRect(item(0)).top() - 2;
    } else if (m_dropInsertRow >= count()) {
        y = visualItemRect(item(count() - 1)).bottom() + 2;
    } else {
        const QRect above = visualItemRect(item(m_dropInsertRow - 1));
        const QRect below = visualItemRect(item(m_dropInsertRow));
        y = (above.bottom() + below.top()) / 2;
    }

    QPainter p(viewport());
    p.setRenderHint(QPainter::Antialiasing, true);

    const int x1 = 10;
    const int x2 = viewport()->width() - 10;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(226, 181, 58, 70));
    p.drawRoundedRect(QRect(x1, y - 5, qMax(8, x2 - x1), 10), 5, 5);

    QPen pen(QColor(226, 181, 58), 3);
    p.setPen(pen);
    p.drawLine(QPoint(x1, y), QPoint(x2, y));
}

void ThumbnailPanel::onThumbnailReady(int generation, int pageIndex, QImage image)
{
    if (generation != m_generation) return;
    if (pageIndex < 0 || pageIndex >= count()) return;

    QImage thumb = image.scaled(THUMB_W, THUMB_H, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    for (int row = 0; row < count(); ++row) {
        QListWidgetItem* it = item(row);
        if (it && it->data(Qt::UserRole).toInt() == pageIndex) {
            it->setIcon(QIcon(QPixmap::fromImage(thumb)));
            break;
        }
    }
}

void ThumbnailPanel::onItemClicked(QListWidgetItem* it)
{
    if (!it) return;
    emit pageSelected(it->data(Qt::UserRole).toInt());
}

void ThumbnailPanel::onContextMenuRequested(const QPoint& pos)
{
    QListWidgetItem* itAtPos = itemAt(pos);

    QList<int> selected;
    for (QListWidgetItem* it : selectedItems()) {
        if (!it) continue;
        selected.append(it->data(Qt::UserRole).toInt());
    }
    if (selected.isEmpty() && itAtPos) {
        selected.append(itAtPos->data(Qt::UserRole).toInt());
    }

    const int pasteAfter = itAtPos ? itAtPos->data(Qt::UserRole).toInt() : (count() - 1);

    QMenu menu(this);
    QAction* actCopy = menu.addAction(tr("Copy"));
    QAction* actCut = menu.addAction(tr("Cut"));
    menu.addSeparator();
    QAction* actRotateLeft = menu.addAction(tr("Rotate Left"));
    QAction* actRotateRight = menu.addAction(tr("Rotate Right"));
    QAction* actDelete = menu.addAction(tr("Delete"));
    menu.addSeparator();
    QAction* actPaste = menu.addAction(tr("Paste"));

    const bool hasSelection = !selected.isEmpty();
    actCopy->setEnabled(hasSelection);
    actCut->setEnabled(hasSelection);
    actRotateLeft->setEnabled(hasSelection && selected.size() == 1);
    actRotateRight->setEnabled(hasSelection && selected.size() == 1);
    actDelete->setEnabled(hasSelection);
    actPaste->setEnabled(true);

    QAction* picked = menu.exec(mapToGlobal(pos));
    if (!picked) {
        return;
    }

    if (picked == actCopy) {
        emit copyRequested(selected);
    } else if (picked == actCut) {
        emit cutRequested(selected);
    } else if (picked == actRotateLeft) {
        emit rotateLeftRequested(selected.first());
    } else if (picked == actRotateRight) {
        emit rotateRightRequested(selected.first());
    } else if (picked == actDelete) {
        for (int i = selected.size() - 1; i >= 0; --i) {
            emit deleteRequested(selected[i]);
        }
    } else if (picked == actPaste) {
        emit pasteRequested(pasteAfter);
    }
}


void ThumbnailPanel::keyPressEvent(QKeyEvent* event)
{
    if (!event) {
        QListWidget::keyPressEvent(event);
        return;
    }

    QList<int> selected;
    for (QListWidgetItem* it : selectedItems()) {
        if (!it) continue;
        selected.append(it->data(Qt::UserRole).toInt());
    }

    if (event->matches(QKeySequence::Copy)) {
        if (!selected.isEmpty()) {
            emit copyRequested(selected);
            event->accept();
            return;
        }
    } else if (event->matches(QKeySequence::Cut)) {
        if (!selected.isEmpty()) {
            emit cutRequested(selected);
            event->accept();
            return;
        }
    } else if (event->matches(QKeySequence::Paste)) {
        int pasteAfter = count() - 1;
        if (QListWidgetItem* cur = currentItem()) {
            pasteAfter = cur->data(Qt::UserRole).toInt();
        }
        emit pasteRequested(pasteAfter);
        event->accept();
        return;
    }

    QListWidget::keyPressEvent(event);
}
