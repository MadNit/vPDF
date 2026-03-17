#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QStackedLayout>

// A panel that can be sliding or collapsible holding different views (Thumbnails, Tools)
class CollapsiblePanel : public QWidget {
    Q_OBJECT

public:
    explicit CollapsiblePanel(QWidget* parent = nullptr);

    void setCentralWidget(QWidget* widget);
    QWidget* currentCentralWidget() const;
    void setTitle(const QString& title);

private:
    QLabel* m_titleLabel;
    QStackedLayout* m_contentLayout;
};
