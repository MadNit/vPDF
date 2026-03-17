#pragma once

#include <QWidget>
#include <QToolButton>
#include <QVBoxLayout>

class SidebarMenu : public QWidget {
    Q_OBJECT

public:
    explicit SidebarMenu(QWidget* parent = nullptr);

signals:
    void pagesClicked();
    void toolsClicked();
    void settingsClicked();

private:
    QToolButton* m_btnPages;
    QToolButton* m_btnTools;
    QToolButton* m_btnSettings;
};
