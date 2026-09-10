/// @file main.cpp
/// @brief Application entry point for Fujinon SX800 Camera Controller.

#include "app-qt/MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QStyleFactory>
#include <glog/logging.h>

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;
    FLAGS_colorlogtostderr = true;

    LOG(INFO) << "Starting Fujinon SX800 Camera Controller (v2.12.0)";

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

    const int retCode = app.exec();
    LOG(INFO) << "Exiting Fujinon SX800 Camera Controller with code " << retCode;
    google::ShutdownGoogleLogging();
    return retCode;
}
