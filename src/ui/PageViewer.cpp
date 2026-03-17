#include "PageViewer.h"

#include <QScrollBar>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QTimer>
#include <QFrame>
#include <QSignalBlocker>
#include <QIcon>
#include <QGridLayout>
#include <QWheelEvent>
#include <QPainter>
#include <QEvent>

PageViewer::PageViewer(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_zoomBar = new QWidget(this);
    m_zoomBar->setStyleSheet("background-color: #FFFFFF; border-top: 1px solid #E0E0E0; border-bottom: 1px solid #C8C8C8;");
    auto* zoomLayout = new QHBoxLayout(m_zoomBar);
    zoomLayout->setContentsMargins(10, 6, 10, 6);
    zoomLayout->setSpacing(8);

    auto* btnZoomOut = new QToolButton(m_zoomBar);
    btnZoomOut->setText("-");
    btnZoomOut->setToolTip(tr("Zoom Out"));

    auto* btnZoomIn = new QToolButton(m_zoomBar);
    btnZoomIn->setText("+");
    btnZoomIn->setToolTip(tr("Zoom In"));

    auto* btnPrevPage = new QToolButton(m_zoomBar);
    btnPrevPage->setIcon(QIcon(":/icons/prev_page.svg"));
    btnPrevPage->setIconSize(QSize(16, 16));
    btnPrevPage->setToolTip(tr("Previous Page"));

    m_pageSpin = new QSpinBox(m_zoomBar);
    m_pageSpin->setRange(1, 1);
    m_pageSpin->setValue(1);
    m_pageSpin->setSuffix("/0");
    m_pageSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_pageSpin->setAlignment(Qt::AlignCenter);
    m_pageSpin->setFixedWidth(84);

    auto* btnNextPage = new QToolButton(m_zoomBar);
    btnNextPage->setIcon(QIcon(":/icons/next_page.svg"));
    btnNextPage->setIconSize(QSize(16, 16));
    btnNextPage->setToolTip(tr("Next Page"));

    m_zoomPercentLabel = new QLabel("100%", m_zoomBar);
    m_zoomPercentLabel->setMinimumWidth(58);
    m_zoomPercentLabel->setAlignment(Qt::AlignCenter);

    auto* btnPctMinus = new QToolButton(m_zoomBar);
    btnPctMinus->setText("-");
    btnPctMinus->setToolTip(tr("Zoom Out"));

    m_zoomSlider = new QSlider(Qt::Horizontal, m_zoomBar);
    m_zoomSlider->setRange(25, 400);
    m_zoomSlider->setSingleStep(5);
    m_zoomSlider->setPageStep(25);
    m_zoomSlider->setFixedWidth(92);

    auto* btnPctPlus = new QToolButton(m_zoomBar);
    btnPctPlus->setText("+");
    btnPctPlus->setToolTip(tr("Zoom In"));

    auto* btnFit = new QToolButton(m_zoomBar);
    btnFit->setIcon(QIcon(":/icons/fit.svg"));
    btnFit->setIconSize(QSize(16, 16));
    btnFit->setToolTip(tr("Fit Width/Height"));

    auto* btnRotateLeft = new QToolButton(m_zoomBar);
    btnRotateLeft->setIcon(QIcon(":/icons/rotate_left.svg"));
    btnRotateLeft->setIconSize(QSize(16, 16));
    btnRotateLeft->setToolTip(tr("Rotate current page left"));

    auto* btnRotateRight = new QToolButton(m_zoomBar);
    btnRotateRight->setIcon(QIcon(":/icons/rotate_right.svg"));
    btnRotateRight->setIconSize(QSize(16, 16));
    btnRotateRight->setToolTip(tr("Rotate current page right"));

    auto* btnTwoPage = new QToolButton(m_zoomBar);
    btnTwoPage->setIcon(QIcon(":/icons/two_page.svg"));
    btnTwoPage->setIconSize(QSize(16, 16));
    btnTwoPage->setToolTip(tr("Two-page View (Toggle)"));
    btnTwoPage->setCheckable(true);

    auto* btnPageByPage = new QToolButton(m_zoomBar);
    btnPageByPage->setIcon(QIcon(":/icons/pages.svg"));
    btnPageByPage->setIconSize(QSize(16, 16));
    btnPageByPage->setToolTip(tr("Page-by-Page Mode"));
    btnPageByPage->setCheckable(true);

    m_searchEdit = new QLineEdit(m_zoomBar);
    m_searchEdit->setPlaceholderText(tr("Search text"));
    m_searchEdit->setFixedWidth(170);

    auto* btnSearchPrev = new QToolButton(m_zoomBar);
    btnSearchPrev->setText("<");
    btnSearchPrev->setToolTip(tr("Previous match"));

    auto* btnSearchNext = new QToolButton(m_zoomBar);
    btnSearchNext->setText(">");
    btnSearchNext->setToolTip(tr("Next match"));

    auto* btnSearchClear = new QToolButton(m_zoomBar);
    btnSearchClear->setText("x");
    btnSearchClear->setToolTip(tr("Clear search"));

    auto* sep1 = new QFrame(m_zoomBar);
    sep1->setFrameShape(QFrame::VLine);
    sep1->setFrameShadow(QFrame::Plain);
    sep1->setLineWidth(2);
    auto* sep2 = new QFrame(m_zoomBar);
    sep2->setFrameShape(QFrame::VLine);
    sep2->setFrameShadow(QFrame::Plain);
    sep2->setLineWidth(2);

    const QString btnStyle =
        "QToolButton { color: #202020; background-color: #F7F7F7; border: 1px solid #C8C8C8; border-radius: 5px; padding: 4px 10px; font-weight: 700; }"
        "QToolButton:hover { background-color: #ECECEC; border-color: #AFAFAF; }"
        "QToolButton:pressed { background-color: #DFDFDF; }"
        "QToolButton:checked { background-color: #E8E8E8; color: #111111; border-color: #8F8F8F; }";
    btnZoomOut->setStyleSheet(btnStyle);
    btnZoomIn->setStyleSheet(btnStyle);
    btnPrevPage->setStyleSheet(btnStyle);
    btnNextPage->setStyleSheet(btnStyle);
    btnPctMinus->setStyleSheet(btnStyle);
    btnPctPlus->setStyleSheet(btnStyle);
    btnFit->setStyleSheet(btnStyle);
    btnRotateLeft->setStyleSheet(btnStyle);
    btnRotateRight->setStyleSheet(btnStyle);
    btnTwoPage->setStyleSheet(btnStyle);
    btnPageByPage->setStyleSheet(btnStyle);
    btnSearchPrev->setStyleSheet(btnStyle);
    btnSearchNext->setStyleSheet(btnStyle);
    btnSearchClear->setStyleSheet(btnStyle);

    m_searchEdit->setStyleSheet(
        "QLineEdit { color: #202020; background-color: #FFFFFF; border: 1px solid #BDBDBD; border-radius: 5px; padding: 3px 8px; }"
        "QLineEdit:focus { border: 1px solid #7E7E7E; }");

    m_pageSpin->setStyleSheet(
        "QSpinBox { color: #202020; font-weight: 700; background-color: #FFFFFF; border: 1px solid #BDBDBD; border-radius: 5px; padding: 2px 6px; }"
        "QSpinBox:focus { border: 1px solid #7E7E7E; }");

    m_zoomPercentLabel->setStyleSheet("QLabel { color: #202020; font-weight: 700; background-color: #FFFFFF; border: 1px solid #BDBDBD; border-radius: 5px; padding: 2px 8px; }");
    m_zoomSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 4px; background: #D0D0D0; border-radius: 2px; }"
        "QSlider::sub-page:horizontal { background: #8A8A8A; border-radius: 2px; }"
        "QSlider::handle:horizontal { width: 12px; margin: -5px 0; border-radius: 6px; background: #FFFFFF; border: 1px solid #7A7A7A; }");
    sep1->setStyleSheet("color: #6A6A6A; min-width: 2px;");
    sep2->setStyleSheet("color: #6A6A6A; min-width: 2px;");

    zoomLayout->addStretch();
    zoomLayout->addWidget(btnZoomOut);
    zoomLayout->addWidget(btnZoomIn);
    zoomLayout->addWidget(sep1);
    zoomLayout->addWidget(btnPrevPage);
    zoomLayout->addWidget(m_pageSpin);
    zoomLayout->addWidget(btnNextPage);
    zoomLayout->addWidget(sep2);
    zoomLayout->addWidget(m_zoomPercentLabel);
    zoomLayout->addWidget(btnPctMinus);
    zoomLayout->addWidget(m_zoomSlider);
    zoomLayout->addWidget(btnPctPlus);
    zoomLayout->addWidget(btnFit);
    zoomLayout->addWidget(btnRotateLeft);
    zoomLayout->addWidget(btnRotateRight);
    zoomLayout->addWidget(btnTwoPage);
    zoomLayout->addWidget(btnPageByPage);
    zoomLayout->addWidget(m_searchEdit);
    zoomLayout->addWidget(btnSearchPrev);
    zoomLayout->addWidget(btnSearchNext);
    zoomLayout->addWidget(btnSearchClear);
    zoomLayout->addStretch();

    mainLayout->addWidget(m_zoomBar);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setBackgroundRole(QPalette::Dark);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_container = new QWidget(m_scrollArea);
    m_layout = new QGridLayout(m_container);
    m_layout->setContentsMargins(8, 10, 8, 10);
    m_layout->setSpacing(12);
    m_layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    m_scrollArea->setWidget(m_container);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->installEventFilter(this);
    m_scrollArea->viewport()->installEventFilter(this);
    mainLayout->addWidget(m_scrollArea);

    connect(m_scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, &PageViewer::onScroll);
    connect(btnZoomOut, &QToolButton::clicked, this, [this]() { zoom(-0.25f); });
    connect(btnZoomIn, &QToolButton::clicked, this, [this]() { zoom(0.25f); });
    connect(btnPrevPage, &QToolButton::clicked, this, [this]() {
        if (m_engine && m_engine->isOpen()) {
            showPage(qMax(0, m_currentPage - 1));
        }
    });
    connect(btnNextPage, &QToolButton::clicked, this, [this]() {
        if (m_engine && m_engine->isOpen()) {
            showPage(qMin(m_engine->pageCount() - 1, m_currentPage + 1));
        }
    });
    connect(btnPctMinus, &QToolButton::clicked, this, [this]() {
        if (m_zoomSlider) {
            m_zoomSlider->setValue(m_zoomSlider->value() - 5);
        }
    });
    connect(btnPctPlus, &QToolButton::clicked, this, [this]() {
        if (m_zoomSlider) {
            m_zoomSlider->setValue(m_zoomSlider->value() + 5);
        }
    });
    connect(m_zoomSlider, &QSlider::valueChanged, this, &PageViewer::onZoomSliderChanged);
    connect(btnFit, &QToolButton::clicked, this, &PageViewer::fitInView);
    connect(btnRotateLeft, &QToolButton::clicked, this, &PageViewer::rotateLeftRequested);
    connect(btnRotateRight, &QToolButton::clicked, this, &PageViewer::rotateRightRequested);
    connect(m_pageSpin, qOverload<int>(&QSpinBox::valueChanged), this, &PageViewer::onPageSpinChanged);

    connect(btnTwoPage, &QToolButton::toggled, this, [this](bool enabled) {
        const int targetColumns = enabled ? 2 : 1;
        if (m_pageColumns != targetColumns) {
            m_pageColumns = targetColumns;
            setupPages();
            showPage(m_currentPage);
        }
    });
    connect(btnPageByPage, &QToolButton::toggled, this, [this, btnTwoPage](bool enabled) {
        m_pageByPageMode = enabled;
        if (enabled) {
            btnTwoPage->setChecked(false);
            if (m_pageColumns != 1) {
                m_pageColumns = 1;
                setupPages();
            }
            btnTwoPage->setEnabled(false);
        } else {
            btnTwoPage->setEnabled(true);
        }
        updatePageVisibility();
        showPage(m_currentPage);
    });

    connect(btnSearchNext, &QToolButton::clicked, this, &PageViewer::onSearchNext);
    connect(btnSearchPrev, &QToolButton::clicked, this, &PageViewer::onSearchPrev);
    connect(btnSearchClear, &QToolButton::clicked, this, &PageViewer::onSearchClear);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &PageViewer::onSearchNext);
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString&) {
        clearSearchError();
        m_lastSearchTerm.clear();
        m_searchHits.clear();
        m_searchHitCursor = -1;
        m_searchRectsByPage.clear();
        if (m_engine && m_engine->isOpen()) {
            refresh();
        }
    });

    updateZoomIndicator();
}

void PageViewer::setEngine(IPdfEngine* engine)
{
    m_engine = engine;
    m_currentPage = 0;
    setupPages();
}

void PageViewer::setupPages()
{
    for (auto* lbl : m_pageLabels) {
        m_layout->removeWidget(lbl);
        delete lbl;
    }
    m_pageLabels.clear();
    m_pageRendered.clear();

    m_searchHits.clear();
    m_lastSearchTerm.clear();
    m_searchHitCursor = -1;
    m_searchRectsByPage.clear();

    if (!m_engine || !m_engine->isOpen()) {
        updatePageIndicator();
        updateZoomIndicator();
        return;
    }

    const int count = m_engine->pageCount();
    for (int i = 0; i < count; ++i) {
        auto* lbl = new QLabel(m_container);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        const int row = i / m_pageColumns;
        const int col = i % m_pageColumns;
        m_layout->addWidget(lbl, row, col, Qt::AlignHCenter | Qt::AlignTop);

        m_pageLabels.push_back(lbl);
        m_pageRendered.push_back(false);
    }

    updateFocusVisuals();
    updatePageVisibility();
    updatePageIndicator();
    updateZoomIndicator();
    refresh();
}

void PageViewer::showPage(int pageIndex)
{
    if (!m_engine || !m_engine->isOpen()) return;
    if (pageIndex < 0 || pageIndex >= m_engine->pageCount()) return;

    m_currentPage = pageIndex;
    updatePageVisibility();
    if (pageIndex < static_cast<int>(m_pageLabels.size())) {
        m_scrollArea->ensureWidgetVisible(m_pageLabels[pageIndex], 0, 50);
    }
    updateFocusVisuals();
    updatePageIndicator();
    emit pageChanged(m_currentPage);
}

void PageViewer::clear()
{
    m_engine = nullptr;
    m_currentPage = 0;
    setupPages();
}

void PageViewer::refresh()
{
    if (!m_engine || !m_engine->isOpen()) return;

    for (int i = 0; i < m_engine->pageCount(); ++i) {
        const QSizeF ptSize = m_engine->pageSize(i);
        const int pxW = qMax(1, static_cast<int>(ptSize.width() * m_dpi / 72.0f));
        const int pxH = qMax(1, static_cast<int>(ptSize.height() * m_dpi / 72.0f));

        m_pageLabels[i]->setFixedSize(pxW, pxH);
        m_pageRendered[i] = false;
        m_pageLabels[i]->clear();
    }

    updateFocusVisuals();
    updatePageVisibility();
    updatePageIndicator();
    updateZoomIndicator();
    QTimer::singleShot(0, this, &PageViewer::renderVisiblePages);
}

void PageViewer::zoom(float delta)
{
    m_dpi = qBound(36.0f, m_dpi + delta * 72.0f, 1440.0f);
    refresh();
}

void PageViewer::fitWidth()
{
    if (!m_engine || !m_engine->isOpen()) return;

    const QSizeF pgSize = m_engine->pageSize(m_currentPage);
    if (pgSize.isEmpty()) return;

    const int viewportW = qMax(10, m_scrollArea->viewport()->width() - 24);
    m_dpi = qBound(10.0f, static_cast<float>(viewportW) / static_cast<float>(pgSize.width()) * 72.0f, 1440.0f);
    refresh();
}

void PageViewer::fitInView()
{
    if (!m_engine || !m_engine->isOpen()) return;

    const QSizeF pgSize = m_engine->pageSize(m_currentPage);
    if (pgSize.isEmpty()) return;

    const int viewportW = qMax(10, m_scrollArea->viewport()->width() - 24);
    const int viewportH = qMax(10, m_scrollArea->viewport()->height() - 60);

    const float dpiW = static_cast<float>(viewportW) / static_cast<float>(pgSize.width()) * 72.0f;
    const float dpiH = static_cast<float>(viewportH) / static_cast<float>(pgSize.height()) * 72.0f;

    m_dpi = qBound(10.0f, qMin(dpiW, dpiH) * 0.95f, 1440.0f);
    refresh();
}

void PageViewer::onScroll()
{
    renderVisiblePages();
}

void PageViewer::onPageSpinChanged(int pageOneBased)
{
    if (!m_engine || !m_engine->isOpen()) {
        return;
    }
    showPage(pageOneBased - 1);
}

void PageViewer::onZoomSliderChanged(int percent)
{
    const float dpi = (static_cast<float>(percent) / 100.0f) * 96.0f;
    m_dpi = qBound(36.0f, dpi, 1440.0f);
    refresh();
}

void PageViewer::onSearchNext()
{
    moveToSearchHit(true);
}

void PageViewer::onSearchPrev()
{
    moveToSearchHit(false);
}

void PageViewer::onSearchClear()
{
    if (!m_searchEdit) {
        return;
    }
    m_searchEdit->clear();
    clearSearchError();
    m_lastSearchTerm.clear();
    m_searchHits.clear();
    m_searchHitCursor = -1;
    m_searchRectsByPage.clear();
    if (m_engine && m_engine->isOpen()) {
        refresh();
    }
}


void PageViewer::clearSearchError()
{
    if (!m_searchEdit) {
        return;
    }
    m_searchEdit->setStyleSheet(
        "QLineEdit { color: #202020; background-color: #FFFFFF; border: 1px solid #BDBDBD; border-radius: 5px; padding: 3px 8px; }"
        "QLineEdit:focus { border: 1px solid #7E7E7E; }");
}

void PageViewer::rebuildSearchHits(const QString& term)
{
    m_searchHits.clear();
    m_searchHitCursor = -1;
    m_searchRectsByPage.clear();

    if (!m_engine || !m_engine->isOpen()) {
        return;
    }

    const QString needle = term.trimmed();
    if (needle.isEmpty()) {
        return;
    }

    const int total = m_engine->pageCount();
    for (int i = 0; i < total; ++i) {
        const QVector<QRectF> rects = m_engine->searchPageTextRects(i, needle);
        if (!rects.isEmpty()) {
            m_searchHits.push_back(i);
            m_searchRectsByPage.insert(i, rects);
        }
    }
}

void PageViewer::moveToSearchHit(bool forward)
{
    if (!m_searchEdit) {
        return;
    }

    const QString term = m_searchEdit->text().trimmed();
    if (term.isEmpty()) {
        clearSearchError();
        return;
    }

    if (term != m_lastSearchTerm) {
        m_lastSearchTerm = term;
        rebuildSearchHits(term);
    }

    if (m_searchHits.isEmpty()) {
        m_searchEdit->setStyleSheet(
            "QLineEdit { color: #202020; background-color: #FFF1F1; border: 1px solid #D45A5A; border-radius: 5px; padding: 3px 8px; }"
            "QLineEdit:focus { border: 1px solid #B33636; }");
        return;
    }

    clearSearchError();

    if (m_searchHitCursor < 0) {
        m_searchHitCursor = 0;
    } else if (forward) {
        m_searchHitCursor = (m_searchHitCursor + 1) % m_searchHits.size();
    } else {
        m_searchHitCursor = (m_searchHitCursor - 1 + m_searchHits.size()) % m_searchHits.size();
    }

    showPage(m_searchHits[m_searchHitCursor]);
}

void PageViewer::drawSearchHighlights(QImage& image, int pageIndex) const
{
    if (!m_engine) {
        return;
    }

    const auto it = m_searchRectsByPage.constFind(pageIndex);
    if (it == m_searchRectsByPage.constEnd() || it->isEmpty()) {
        return;
    }

    const QSizeF pagePt = m_engine->pageSize(pageIndex);
    if (pagePt.width() <= 0.0 || pagePt.height() <= 0.0) {
        return;
    }

    const double sx = static_cast<double>(image.width()) / pagePt.width();
    const double sy = static_cast<double>(image.height()) / pagePt.height();

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(92, 160, 255, 210), 1));
    painter.setBrush(QColor(135, 206, 255, 95));

    for (const QRectF& ptRect : *it) {
        if (ptRect.width() <= 0.0 || ptRect.height() <= 0.0) {
            continue;
        }
        const double x = ptRect.x() * sx;
        const double w = ptRect.width() * sx;
        const double h = ptRect.height() * sy;
        const double y = (pagePt.height() - (ptRect.y() + ptRect.height())) * sy;
        painter.drawRoundedRect(QRectF(x, y, w, h), 1.5, 1.5);
    }
}

void PageViewer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    renderVisiblePages();
}

bool PageViewer::eventFilter(QObject* watched, QEvent* event)
{
    const bool isWheelTarget = m_scrollArea && (watched == m_scrollArea || watched == m_scrollArea->viewport());
    if (isWheelTarget && event && event->type() == QEvent::Wheel) {
        auto* wheel = static_cast<QWheelEvent*>(event);
        int dy = wheel->angleDelta().y();
        if (dy == 0) {
            dy = wheel->pixelDelta().y();
        }

        if ((wheel->modifiers() & Qt::ControlModifier) && dy != 0) {
            if (dy > 0) {
                zoom(+0.10f);
            } else {
                zoom(-0.10f);
            }
            wheel->accept();
            return true;
        }

        if (m_pageByPageMode && m_engine && m_engine->isOpen() && dy != 0) {
            if (dy > 0) {
                showPage(qMax(0, m_currentPage - 1));
            } else {
                showPage(qMin(m_engine->pageCount() - 1, m_currentPage + 1));
            }
            wheel->accept();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void PageViewer::wheelEvent(QWheelEvent* event)
{
    if (event && (event->modifiers() & Qt::ControlModifier)) {
        int dy = event->angleDelta().y();
        if (dy == 0) {
            dy = event->pixelDelta().y();
        }
        if (dy > 0) {
            zoom(+0.10f);
            event->accept();
            return;
        } else if (dy < 0) {
            zoom(-0.10f);
            event->accept();
            return;
        }
    }

    if (event && m_pageByPageMode && m_engine && m_engine->isOpen()) {
        int dy = event->angleDelta().y();
        if (dy == 0) {
            dy = event->pixelDelta().y();
        }
        if (dy > 0) {
            showPage(qMax(0, m_currentPage - 1));
            event->accept();
            return;
        }
        if (dy < 0) {
            showPage(qMin(m_engine->pageCount() - 1, m_currentPage + 1));
            event->accept();
            return;
        }
    }

    QWidget::wheelEvent(event);
}

void PageViewer::renderVisiblePages()
{
    if (!m_engine || !m_engine->isOpen()) return;

    if (m_pageByPageMode) {
        if (m_currentPage >= 0 && m_currentPage < static_cast<int>(m_pageLabels.size()) && !m_pageRendered[m_currentPage]) {
            QImage img = m_engine->renderPage(m_currentPage, m_dpi);
            if (!img.isNull()) {
                drawSearchHighlights(img, m_currentPage);
                m_pageLabels[m_currentPage]->setPixmap(QPixmap::fromImage(img));
            }
            m_pageRendered[m_currentPage] = true;
        }
        return;
    }

    const QRect viewportRect = m_scrollArea->viewport()->rect();
    const QRect renderRect = viewportRect.adjusted(0, -500, 0, 500);

    int bestPage = m_currentPage;
    int maxIntersect = 0;

    for (size_t i = 0; i < m_pageLabels.size(); ++i) {
        QLabel* lbl = m_pageLabels[i];
        const QRect lblRect(lbl->mapTo(m_scrollArea->viewport(), QPoint(0, 0)), lbl->size());

        const int intersectHeight = viewportRect.intersected(lblRect).height();
        if (intersectHeight > maxIntersect) {
            maxIntersect = intersectHeight;
            bestPage = static_cast<int>(i);
        }

        if (renderRect.intersects(lblRect)) {
            if (!m_pageRendered[i]) {
                QImage img = m_engine->renderPage(static_cast<int>(i), m_dpi);
                if (!img.isNull()) {
                    drawSearchHighlights(img, static_cast<int>(i));
                    lbl->setPixmap(QPixmap::fromImage(img));
                }
                m_pageRendered[i] = true;
            }
        } else if (m_pageRendered[i] && !viewportRect.adjusted(0, -1000, 0, 1000).intersects(lblRect)) {
            lbl->clear();
            m_pageRendered[i] = false;
        }
    }

    if (bestPage != m_currentPage) {
        m_currentPage = bestPage;
        updateFocusVisuals();
        updatePageIndicator();
        emit pageChanged(m_currentPage);
    }
}

void PageViewer::updateFocusVisuals()
{
    for (int i = 0; i < static_cast<int>(m_pageLabels.size()); ++i) {
        if (i == m_currentPage) {
            m_pageLabels[i]->setStyleSheet("QLabel { background-color: white; border: 2px solid #FF5A5A; border-radius: 2px; }");
        } else {
            m_pageLabels[i]->setStyleSheet("QLabel { background-color: white; border: 1px solid #2A2A2A; border-radius: 2px; }");
        }
    }
}

void PageViewer::updatePageVisibility()
{
    for (int i = 0; i < static_cast<int>(m_pageLabels.size()); ++i) {
        const bool visible = !m_pageByPageMode || (i == m_currentPage);
        m_pageLabels[i]->setVisible(visible);
    }
}

void PageViewer::updatePageIndicator()
{
    if (!m_pageSpin) {
        return;
    }

    if (!m_engine || !m_engine->isOpen()) {
        const QSignalBlocker blocker(m_pageSpin);
        m_pageSpin->setRange(1, 1);
        m_pageSpin->setValue(1);
        m_pageSpin->setSuffix("/0");
        return;
    }

    const int total = m_engine->pageCount();
    const int current = qBound(0, m_currentPage, qMax(0, total - 1));

    {
        const QSignalBlocker blocker(m_pageSpin);
        m_pageSpin->setRange(1, qMax(1, total));
        m_pageSpin->setValue(current + 1);
        m_pageSpin->setSuffix(QString("/%1").arg(total));
    }
}

void PageViewer::updateZoomIndicator()
{
    if (!m_zoomPercentLabel) return;

    const int pct = qRound((m_dpi / 96.0f) * 100.0f);
    m_zoomPercentLabel->setText(QString("%1%").arg(pct));

    if (m_zoomSlider) {
        const QSignalBlocker blocker(m_zoomSlider);
        m_zoomSlider->setValue(qBound(m_zoomSlider->minimum(), pct, m_zoomSlider->maximum()));
    }
}


