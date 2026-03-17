#include "SidebarMenu.h"
#include <QIcon>
#include <QDebug>
#include <QSpacerItem>

SidebarMenu::SidebarMenu(QWidget* parent) : QWidget(parent) {
    setFixedWidth(56);
    setObjectName("sidebarMenu");
    setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 16, 0, 16);
    layout->setSpacing(12);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // Ensure sidebar background + border are painted from first render,
    // and keep icon SVGs visible (they use stroke="currentColor").
    setStyleSheet(
        "QWidget#sidebarMenu { background-color: #E6E6E6; border-right: 1px solid #9A9A9A; }"
        "QWidget#sidebarMenu QToolButton { color: #111111; background-color: transparent; border: 1px solid #6E6E6E; border-radius: 6px; padding: 8px; margin: 4px 6px; }"
        "QWidget#sidebarMenu QToolButton:hover { background-color: #D5D5D5; border-color: #4F4F4F; }"
        "QWidget#sidebarMenu QToolButton:checked, QWidget#sidebarMenu QToolButton:pressed { background-color: #C8C8C8; color: #111111; border-color: #3E3E3E; }"
    );

    auto createButton = [](const QString& iconName, const QString& tooltip) {
        auto* btn = new QToolButton();
        btn->setIcon(QIcon(iconName));
        btn->setIconSize(QSize(24, 24));
        btn->setToolTip(tooltip);
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        return btn;
    };

    m_btnPages = createButton(":/icons/pages.svg", tr("Pages"));
    m_btnTools = createButton(":/icons/tools.svg", tr("Tools"));

    auto* spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    m_btnSettings = createButton(":/icons/settings.svg", tr("Settings"));
    m_btnSettings->setAutoExclusive(false);
    m_btnSettings->setCheckable(false);

    layout->addWidget(m_btnPages);
    layout->addWidget(m_btnTools);
    layout->addItem(spacer);
    layout->addWidget(m_btnSettings);

    connect(m_btnPages, &QToolButton::clicked, this, &SidebarMenu::pagesClicked);
    connect(m_btnTools, &QToolButton::clicked, this, &SidebarMenu::toolsClicked);
    connect(m_btnSettings, &QToolButton::clicked, this, &SidebarMenu::settingsClicked);
}


