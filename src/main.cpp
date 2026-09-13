#include <QApplication>
#include <QCoreApplication>
#include "MainWindow.h"
#include "ReaderSessionController.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("AstraPDF");
    QCoreApplication::setApplicationName("AstraPDF");
    QCoreApplication::setApplicationVersion("0.2.0");

    MainWindow w;
    ReaderSessionController sessions(&w,&w);
    w.show();

    if(argc>1)
        sessions.openTracked(QString::fromLocal8Bit(argv[1]));

    return app.exec();
}
