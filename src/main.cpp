#include <QApplication>
#include <QCoreApplication>
#include "MainWindow.h"
#include "ReaderSessionController.h"
#include "UiEnhancer.h"
#include "BackgroundController.h"
#include "InterfaceController.h"
#include "EdgeChromeController.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("AstraPDF");
    QCoreApplication::setApplicationName("AstraPDF");
    QCoreApplication::setApplicationVersion("0.2.0");

    MainWindow w;
    ReaderSessionController sessions(&w,&w);
    UiEnhancer enhancer(&w,&w);
    BackgroundController backgrounds(&w,&w);
    InterfaceController interfaceController(&w,&w);
    EdgeChromeController edgeChrome(&w,&w);
    w.show();

    if(argc>1)
        sessions.openTracked(QString::fromLocal8Bit(argv[1]));

    return app.exec();
}
