/// @file main.cpp
/// @brief Application entry point for Fujinon SX800 Camera Controller.

#include "app-qt/MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QStyleFactory>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("FujinonSX800Controller");
    app.setApplicationVersion("2.12.0");
    app.setOrganizationName("FUJIFILM");

    // Load dark stylesheet
    QFile qssFile(":/resources/ThemeDark.qss");
    if (!qssFile.exists()) {
        qssFile.setFileName("app-qt/resources/ThemeDark.qss");
    }
    if (qssFile.open(QFile::ReadOnly | QFile::Text)) {
        const QString stylesheet = QString::fromUtf8(qssFile.readAll());
        app.setStyleSheet(stylesheet);
        qssFile.close();
    }

    FujinonSX800App::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
