#include <QApplication>
#include "MainWindow.h"
#include "ThemeConfig.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Vole Machine Simulator");
    app.setApplicationVersion("1.0.0");

    const QString styleSheet = ThemeConfig::loadStyleSheet();
    if (!styleSheet.isEmpty()) {
        app.setStyleSheet(styleSheet);
    }

    MainWindow window;
    window.show();

    return app.exec();
}
