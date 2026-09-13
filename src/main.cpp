#include <QApplication>
#include <QCoreApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("AstraPDF");
    QCoreApplication::setApplicationName("AstraPDF");
    QCoreApplication::setApplicationVersion("0.2.0");
    MainWindow w;
    w.show();
    if (argc > 1) w.openPdf(QString::fromLocal8Bit(argv[1]));
    return app.exec();
}
