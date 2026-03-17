#include "CollapsiblePanel.h"

#include <QHBoxLayout>

CollapsiblePanel::CollapsiblePanel(QWidget* parent) : QWidget(parent) {
    setMinimumWidth(120);
    setMaximumWidth(200);
    setObjectName("collapsiblePanel");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Dedicated, fixed-height header to prevent content overlap.
    auto* titleBar = new QWidget(this);
    titleBar->setObjectName("collapsiblePanelTitleBar");
    titleBar->setFixedHeight(44);

    auto* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(12, 10, 12, 10);
    titleLayout->setSpacing(0);

    m_titleLabel = new QLabel("Panel", titleBar);
    m_titleLabel->setObjectName("collapsiblePanelTitleLabel");
    m_titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_titleLabel->setWordWrap(false);
    titleLayout->addWidget(m_titleLabel, 1);

    auto* contentContainer = new QWidget(this);
    contentContainer->setObjectName("collapsiblePanelContent");
    m_contentLayout = new QStackedLayout(contentContainer);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);

    mainLayout->addWidget(titleBar);
    mainLayout->addWidget(contentContainer, 1);

    setStyleSheet(
        "QWidget#collapsiblePanel { background-color: #161616; border-right: 1px solid #2A2A2A; }"
        "QWidget#collapsiblePanelTitleBar { background-color: #1A1A1A; border-bottom: 1px solid #2A2A2A; }"
        "QLabel#collapsiblePanelTitleLabel { color: #F1F1F1; font-weight: 700; font-size: 12px; background: transparent; }"
        "QWidget#collapsiblePanelContent { background-color: #161616; }"
    );

    // Keep header painted above any stacked content.
    titleBar->raise();
}

void CollapsiblePanel::setCentralWidget(QWidget* widget) {
    if (!widget) {
        return;
    }

    if (m_contentLayout->indexOf(widget) < 0) {
        m_contentLayout->addWidget(widget);
    }
    m_contentLayout->setCurrentWidget(widget);
}

QWidget* CollapsiblePanel::currentCentralWidget() const {
    return m_contentLayout->currentWidget();
}

void CollapsiblePanel::setTitle(const QString& title) {
    m_titleLabel->setText(title);
}
