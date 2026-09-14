#include <QApplication>
#include <QCoreApplication>
#include "MainWindow.h"
#include "ReaderSessionController.h"
#include "UiEnhancer.h"
#include "BackgroundController.h"
#include "InterfaceController.h"
#include "RailPolishController.h"
#include "TargetedRailFixController.h"

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
    RailPolishController railPolish(&w,&w);
    TargetedRailFixController targetedRailFix(&w,&w);

    // RailPolish owns cursor-driven rail visibility. Prevent the older fullscreen
    // chrome event filter from repeatedly showing the toolbar on every mouse move.
    qApp->removeEventFilter(&sessions);

    w.show();

    if(argc>1)
        sessions.openTracked(QString::fromLocal8Bit(argv[1]));

    return app.exec();
}
