#include "ReaderSessionController.h"
#include "MainWindow.h"
#include "PdfCanvas.h"

#include <QAction>
#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDockWidget>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPdfDocument>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <algorithm>

ReaderSessionController::ReaderSessionController(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    m_canvas=m_window->findChild<PdfCanvas*>();
    m_document=m_window->findChild<QPdfDocument*>();
    m_readerToolbar=m_window->findChild<QToolBar*>("readerToolbar");

    buildHistoryDock();
    setupOpenActionTracking();

    m_chromeTimer=new QTimer(this);
    m_chromeTimer->setSingleShot(true);
    m_chromeTimer->setInterval(2200);
    connect(m_chromeTimer,&QTimer::timeout,this,&ReaderSessionController::hideReaderChrome);

    if(m_canvas){
        m_canvas->setMouseTracking(true);
        connect(m_canvas,&PdfCanvas::currentPageChanged,this,[this](int page){
            if(!m_restoringPage) recordPage(page);
        });
    }

    if(auto *scroll=m_window->findChild<QScrollArea*>()){
        scroll->setMouseTracking(true);
        if(scroll->viewport()) scroll->viewport()->setMouseTracking(true);
    }
    m_window->setMouseTracking(true);
    qApp->installEventFilter(this);

    if(m_document){
        connect(m_document,&QPdfDocument::statusChanged,this,[this](QPdfDocument::Status status){
            if(status!=QPdfDocument::Status::Ready || !m_canvas || m_currentPath.isEmpty()) return;
            const int count=m_document->pageCount();
            if(count<=0) return;
            const int page=std::clamp(m_pendingRestorePage,0,count-1);
            m_restoringPage=true;
            m_canvas->setCurrentPage(page);
            m_restoringPage=false;
            recordPage(page);
            m_pendingRestorePage=-1;
            setReaderFullScreen(true);
        });
    }

    refreshHistory();
}

QString ReaderSessionController::normalizedPath(const QString& filePath) const
{
    return QFileInfo(filePath).absoluteFilePath();
}

QString ReaderSessionController::historyKey(const QString& filePath) const
{
    const QByteArray data=normalizedPath(filePath).toLower().toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(data,QCryptographicHash::Sha1).toHex());
}

int ReaderSessionController::savedPage(const QString& filePath) const
{
    QSettings s;
    s.beginGroup(QStringLiteral("history/items/%1").arg(historyKey(filePath)));
    const int page=s.value("page",0).toInt();
    s.endGroup();
    return std::max(0,page);
}

void ReaderSessionController::recordOpened(const QString& filePath)
{
    const QString path=normalizedPath(filePath);
    QSettings s;
    QStringList paths=s.value("history/paths").toStringList();
    paths.removeAll(path);
    paths.prepend(path);
    s.setValue("history/paths",paths);

    s.beginGroup(QStringLiteral("history/items/%1").arg(historyKey(path)));
    s.setValue("path",path);
    if(!s.contains("page")) s.setValue("page",0);
    s.setValue("lastOpened",QDateTime::currentDateTime().toString(Qt::ISODate));
    s.endGroup();
    s.sync();
}

void ReaderSessionController::recordPage(int page)
{
    if(m_currentPath.isEmpty()) return;
    QSettings s;
    s.beginGroup(QStringLiteral("history/items/%1").arg(historyKey(m_currentPath)));
    s.setValue("path",m_currentPath);
    s.setValue("page",std::max(0,page));
    if(!s.contains("lastOpened")) s.setValue("lastOpened",QDateTime::currentDateTime().toString(Qt::ISODate));
    s.endGroup();
    s.sync();
    refreshHistory();
}

void ReaderSessionController::refreshHistory()
{
    if(!m_historyList) return;
    m_historyList->clear();
    QSettings s;
    const QStringList paths=s.value("history/paths").toStringList();
    for(const QString& path:paths){
        s.beginGroup(QStringLiteral("history/items/%1").arg(historyKey(path)));
        const int page=s.value("page",0).toInt();
        const QString opened=s.value("lastOpened").toString();
        s.endGroup();

        const QFileInfo info(path);
        const QString when=QDateTime::fromString(opened,Qt::ISODate).isValid()
            ? QDateTime::fromString(opened,Qt::ISODate).toString("yyyy-MM-dd HH:mm")
            : QStringLiteral("unknown time");
        const QString missing=info.exists()?QString():QStringLiteral("  [missing]");
        auto *item=new QListWidgetItem(QStringLiteral("%1\nPage %2  •  %3%4")
                                       .arg(info.fileName().isEmpty()?path:info.fileName())
                                       .arg(page+1)
                                       .arg(when,missing),m_historyList);
        item->setData(Qt::UserRole,path);
        item->setToolTip(path);
    }
}

void ReaderSessionController::buildHistoryDock()
{
    m_historyDock=new QDockWidget("Reading History",m_window);
    m_historyDock->setObjectName("historyDock");
    m_historyDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);

    auto *panel=new QWidget(m_historyDock);
    auto *layout=new QVBoxLayout(panel);
    layout->setContentsMargins(10,10,10,10);
    layout->setSpacing(8);

    auto *hint=new QLabel("Every opened PDF is kept here with its last page. History survives app restarts.",panel);
    hint->setWordWrap(true);
    layout->addWidget(hint);

    m_historyList=new QListWidget(panel);
    m_historyList->setAlternatingRowColors(true);
    m_historyList->setWordWrap(true);
    layout->addWidget(m_historyList,1);

    auto *openButton=new QPushButton("Open selected",panel);
    layout->addWidget(openButton);

    m_historyDock->setWidget(panel);
    m_window->addDockWidget(Qt::LeftDockWidgetArea,m_historyDock);
    m_historyDock->hide();

    m_historyAction=new QAction("History",this);
    m_historyAction->setShortcut(QKeySequence("Ctrl+Shift+H"));
    connect(m_historyAction,&QAction::triggered,this,[this]{
        m_historyDock->show();
        m_historyDock->raise();
        showReaderChrome();
        restartChromeTimer();
    });

    if(m_readerToolbar){
        m_readerToolbar->addSeparator();
        m_readerToolbar->addAction(m_historyAction);

        auto *minimize=new QAction(m_window->style()->standardIcon(QStyle::SP_TitleBarMinButton),"Min",this);
        auto *windowMode=new QAction(m_window->style()->standardIcon(QStyle::SP_TitleBarMaxButton),"Window",this);
        auto *closeAction=new QAction(m_window->style()->standardIcon(QStyle::SP_TitleBarCloseButton),"Close",this);
        connect(minimize,&QAction::triggered,m_window,&QWidget::showMinimized);
        connect(windowMode,&QAction::triggered,this,[this]{ setReaderFullScreen(!m_readerFullScreen); });
        connect(closeAction,&QAction::triggered,m_window,&QWidget::close);
        m_readerToolbar->addSeparator();
        m_readerToolbar->addAction(minimize);
        m_readerToolbar->addAction(windowMode);
        m_readerToolbar->addAction(closeAction);
    }

    for(QMenu *menu:m_window->findChildren<QMenu*>()){
        const QString title=menu->title();
        if(title.contains("File",Qt::CaseInsensitive)){
            menu->addSeparator();
            menu->addAction(m_historyAction);
            break;
        }
    }

    m_fullScreenAction=new QAction("Immersive Full Screen",this);
    m_fullScreenAction->setShortcut(QKeySequence(Qt::Key_F11));
    m_fullScreenAction->setCheckable(true);
    connect(m_fullScreenAction,&QAction::toggled,this,&ReaderSessionController::setReaderFullScreen);
    m_window->addAction(m_fullScreenAction);

    for(QMenu *menu:m_window->findChildren<QMenu*>()){
        if(menu->title().contains("View",Qt::CaseInsensitive)){
            menu->addSeparator();
            menu->addAction(m_fullScreenAction);
            break;
        }
    }

    auto openSelected=[this]{
        if(!m_historyList || !m_historyList->currentItem()) return;
        openTracked(m_historyList->currentItem()->data(Qt::UserRole).toString());
    };
    connect(openButton,&QPushButton::clicked,this,openSelected);
    connect(m_historyList,&QListWidget::itemDoubleClicked,this,[this](QListWidgetItem *item){
        if(item) openTracked(item->data(Qt::UserRole).toString());
    });
}

void ReaderSessionController::setupOpenActionTracking()
{
    const auto actions=m_window->findChildren<QAction*>();
    for(QAction *action:actions){
        if(action->shortcut()!=QKeySequence::Open && action->text()!="Open PDF...") continue;
        QObject::disconnect(action,nullptr,m_window,nullptr);
        connect(action,&QAction::triggered,this,[this]{
            const QString path=QFileDialog::getOpenFileName(m_window,"Open PDF",QString(),"PDF documents (*.pdf);;All files (*.*)");
            if(!path.isEmpty()) openTracked(path);
        });
        break;
    }
}

void ReaderSessionController::openTracked(const QString& filePath)
{
    const QString path=normalizedPath(filePath);
    if(!QFileInfo::exists(path)){
        QMessageBox::warning(m_window,"Open PDF",QStringLiteral("File not found:\n%1").arg(path));
        return;
    }
    m_currentPath=path;
    m_pendingRestorePage=savedPage(path);
    recordOpened(path);
    refreshHistory();
    m_window->openPdf(path);
}

void ReaderSessionController::setReaderFullScreen(bool enabled)
{
    if(!m_window) return;
    m_readerFullScreen=enabled;
    if(m_fullScreenAction && m_fullScreenAction->isChecked()!=enabled){
        const QSignalBlocker blocker(m_fullScreenAction);
        m_fullScreenAction->setChecked(enabled);
    }

    if(enabled){
        m_window->showFullScreen();
        showReaderChrome();
        restartChromeTimer();
    }else{
        if(m_chromeTimer) m_chromeTimer->stop();
        showReaderChrome();
        m_window->showMaximized();
    }
}

void ReaderSessionController::showReaderChrome()
{
    if(!m_window) return;
    m_window->menuBar()->show();
    if(m_readerToolbar) m_readerToolbar->show();
    if(m_window->statusBar()) m_window->statusBar()->show();

    const auto docks=m_window->findChildren<QDockWidget*>();
    for(QDockWidget *dock:docks){
        if(m_visibleDockNames.contains(dock->objectName())) dock->show();
    }
}

void ReaderSessionController::hideReaderChrome()
{
    if(!m_readerFullScreen || !m_window) return;
    m_visibleDockNames.clear();
    const auto docks=m_window->findChildren<QDockWidget*>();
    for(QDockWidget *dock:docks){
        if(dock->isVisible()) m_visibleDockNames<<dock->objectName();
        dock->hide();
    }
    m_window->menuBar()->hide();
    if(m_readerToolbar) m_readerToolbar->hide();
    if(m_window->statusBar()) m_window->statusBar()->hide();
}

void ReaderSessionController::restartChromeTimer()
{
    if(m_readerFullScreen && m_chromeTimer) m_chromeTimer->start();
}

bool ReaderSessionController::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    if(!m_readerFullScreen) return QObject::eventFilter(watched,event);

    switch(event->type()){
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::Wheel:
        showReaderChrome();
        restartChromeTimer();
        break;
    case QEvent::KeyPress:{
        auto *keyEvent=static_cast<QKeyEvent*>(event);
        if(keyEvent->key()==Qt::Key_Escape){
            setReaderFullScreen(false);
            return true;
        }
        showReaderChrome();
        restartChromeTimer();
        break;
    }
    default:
        break;
    }
    return QObject::eventFilter(watched,event);
}
