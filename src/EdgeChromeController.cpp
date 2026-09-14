#include "EdgeChromeController.h"
#include "MainWindow.h"

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QMenuBar>
#include <QTimer>
#include <QToolBar>

EdgeChromeController::EdgeChromeController(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    if(!m_window) return;
    m_toolbar=m_window->findChild<QToolBar*>("readerToolbar");
    qApp->installEventFilter(this);
    QTimer::singleShot(0,this,&EdgeChromeController::syncChrome);
}

void EdgeChromeController::syncChrome()
{
    if(!m_window || !m_toolbar) return;

    if(m_window->menuBar()) m_window->menuBar()->hide();

    m_toolbar->setFixedHeight(qMax(300,m_window->height()-12));
    m_toolbar->move(3,6);

    const QPoint global=QCursor::pos();
    const QPoint local=m_window->mapFromGlobal(global);
    const bool insideWindow=m_window->rect().contains(local);
    const bool atLeftEdge=insideWindow && local.x()>=0 && local.x()<=7;

    const QPoint railTopLeft=m_toolbar->mapToGlobal(QPoint(0,0));
    const QRect railRect(railTopLeft,m_toolbar->size());
    const bool overRail=m_toolbar->isVisible() && railRect.adjusted(-3,-3,8,3).contains(global);
    const bool popupOpen=QApplication::activePopupWidget()!=nullptr;

    if(atLeftEdge || overRail || popupOpen){
        m_toolbar->show();
        m_toolbar->raise();
    }else{
        m_toolbar->hide();
    }
}

bool EdgeChromeController::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    switch(event->type()){
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::Wheel:
    case QEvent::Resize:
        QTimer::singleShot(0,this,&EdgeChromeController::syncChrome);
        break;
    default:
        break;
    }
    return QObject::eventFilter(watched,event);
}
