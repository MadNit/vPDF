#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QWheelEvent>
#include <QLineEdit>
#include <QVector>
#include <QHash>
#include <vector>

#include "engine/IPdfEngine.h"

class PageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit PageViewer(QWidget* parent = nullptr);

    void setEngine(IPdfEngine* engine);
    void showPage(int pageIndex);
    void clear();
    void refresh();
    void zoom(float delta);
    void fitWidth();
    void fitInView();

    int currentPage() const { return m_currentPage; }
    float currentDpi() const { return m_dpi; }

signals:
    void pageChanged(int pageIndex);
    void rotateLeftRequested();
    void rotateRightRequested();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onScroll();
    void onPageSpinChanged(int pageOneBased);
    void onZoomSliderChanged(int percent);
    void onSearchNext();
    void onSearchPrev();
    void onSearchClear();

private:
    void setupPages();
    void renderVisiblePages();
    void updateFocusVisuals();
    void updatePageVisibility();
    void updatePageIndicator();
    void updateZoomIndicator();
    void rebuildSearchHits(const QString& term);
    void moveToSearchHit(bool forward);
    void clearSearchError();
    void drawSearchHighlights(QImage& image, int pageIndex) const;

    IPdfEngine* m_engine = nullptr;
    QWidget* m_zoomBar = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_container = nullptr;

    QSpinBox* m_pageSpin = nullptr;
    QLabel* m_zoomPercentLabel = nullptr;
    QSlider* m_zoomSlider = nullptr;
    QLineEdit* m_searchEdit = nullptr;

    class QGridLayout* m_layout = nullptr;
    std::vector<class QLabel*> m_pageLabels;
    std::vector<bool> m_pageRendered;

    int m_pageColumns = 1;
    int m_currentPage = 0;
    float m_dpi = 96.0f;
    bool m_pageByPageMode = false;

    QVector<int> m_searchHits;
    QString m_lastSearchTerm;
    int m_searchHitCursor = -1;
    QHash<int, QVector<QRectF>> m_searchRectsByPage;
};


