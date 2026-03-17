#include <QApplication>
#include <QStyleFactory>
#include "app/MainWindow.h"
#include <QStyleFactory>
#include <QFile>
#include <QTextStream>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("PDFKill");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("meic");
    
    // Use Fusion base to ensure QSS applies uniformly over native Windows menus
    app.setStyle(QStyleFactory::create("Fusion"));

    // Apply custom dark theme
    QFile styleFile(":/theme/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&styleFile);
        app.setStyleSheet(ts.readAll());
        styleFile.close();
    }

    MainWindow window;
    window.resize(1280, 800);
    window.show();

    return app.exec();
}
