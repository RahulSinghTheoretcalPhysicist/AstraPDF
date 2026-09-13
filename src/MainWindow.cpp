#include "MainWindow.h"
#include "PdfCanvas.h"
#include "CosInspector.h"
#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPdfDocument>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent),
    m_document(new QPdfDocument(this)),m_scrollArea(new QScrollArea(this)),m_canvas(new PdfCanvas)
{
    buildUi();
    buildActions();
    buildToolbar();
    m_canvas->setDocument(m_document);

    connect(m_canvas,&PdfCanvas::currentPageChanged,this,&MainWindow::updatePageUi);
    connect(m_canvas,&PdfCanvas::requestEnsureVisible,this,[this](const QRectF& r){
        m_scrollArea->ensureVisible(int(r.center().x()),int(r.center().y()),
                                   int(std::max<qreal>(40.0,r.width()/2.0)),
                                   int(std::max<qreal>(40.0,r.height()/2.0)));
    });
    connect(m_canvas,&PdfCanvas::statusMessage,this,[this](const QString& s){
        statusBar()->showMessage(s,4000);
    });
    connect(m_document,&QPdfDocument::statusChanged,this,[this](QPdfDocument::Status status){
        if(status==QPdfDocument::Status::Ready){
            finishOpenPdf();
        }else if(status==QPdfDocument::Status::Error){
            const int code=static_cast<int>(m_document->error());
            QMessageBox::critical(this,"Open PDF",QStringLiteral("Could not open PDF. Error code: %1").arg(code));
            m_currentFile.clear();
            statusBar()->showMessage("Failed to open PDF.",5000);
        }
    });

    resize(1360,900);
}

void MainWindow::buildUi()
{
    setObjectName("mainWindow");
    m_scrollArea->setObjectName("documentArea");
    m_scrollArea->setWidget(m_canvas);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setAlignment(Qt::AlignLeft|Qt::AlignTop);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    setCentralWidget(m_scrollArea);
    setWindowTitle("AstraPDF");

    setStyleSheet(R"(
        QMainWindow#mainWindow {
            background: #15181d;
        }
        QMenuBar {
            background: #1b1f26;
            color: #e8edf3;
            border-bottom: 1px solid #2b313b;
            padding: 3px 6px;
        }
        QMenuBar::item {
            padding: 6px 10px;
            border-radius: 5px;
        }
        QMenuBar::item:selected {
            background: #2a3039;
        }
        QMenu {
            background: #20252d;
            color: #edf1f5;
            border: 1px solid #343b46;
            padding: 6px;
        }
        QMenu::item {
            padding: 7px 28px 7px 10px;
            border-radius: 5px;
        }
        QMenu::item:selected {
            background: #343b46;
        }
        QToolBar {
            background: #1b1f26;
            border: none;
            border-bottom: 1px solid #2b313b;
            spacing: 6px;
            padding: 8px 10px;
        }
        QToolBar::separator {
            background: #343a43;
            width: 1px;
            margin: 6px 5px;
        }
        QToolButton {
            color: #e7ecf2;
            background: #272d36;
            border: 1px solid #373f4a;
            border-radius: 7px;
            padding: 7px 10px;
            min-height: 20px;
        }
        QToolButton:hover {
            background: #343c48;
            border-color: #566170;
        }
        QToolButton:pressed {
            background: #171b20;
        }
        QToolButton[openPdfButton="true"] {
            background: #2f6feb;
            border-color: #3979f2;
            color: white;
            font-weight: 600;
            padding-left: 14px;
            padding-right: 14px;
        }
        QToolButton[openPdfButton="true"]:hover {
            background: #3b7cf2;
        }
        QSpinBox, QComboBox, QLineEdit {
            background: #11151a;
            color: #edf1f5;
            border: 1px solid #39414d;
            border-radius: 7px;
            padding: 6px 8px;
            min-height: 22px;
            selection-background-color: #2f6feb;
        }
        QSpinBox:hover, QComboBox:hover, QLineEdit:hover {
            border-color: #5a6675;
        }
        QSpinBox:focus, QComboBox:focus, QLineEdit:focus {
            border: 1px solid #4d8cff;
        }
        QComboBox::drop-down {
            border: none;
            width: 24px;
        }
        QLabel {
            color: #cbd2dc;
        }
        QScrollArea#documentArea {
            background: #111419;
        }
        QStatusBar {
            background: #1b1f26;
            color: #aeb7c4;
            border-top: 1px solid #2b313b;
        }
    )");

    statusBar()->showMessage("Ready  •  Open a PDF with the blue button or press Ctrl+O");
}

void MainWindow::buildActions()
{
    m_open=new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton),"Open PDF...",this);
    m_open->setShortcut(QKeySequence::Open);
    m_open->setToolTip("Open a PDF document (Ctrl+O)");
    connect(m_open,&QAction::triggered,this,[this]{
        QString p=QFileDialog::getOpenFileName(this,"Open PDF",QString(),"PDF documents (*.pdf);;All files (*.*)");
        if(!p.isEmpty())openPdf(p);
    });

    m_copy=new QAction(style()->standardIcon(QStyle::SP_FileIcon),"Copy",this);
    m_copy->setShortcut(QKeySequence::Copy);
    m_copy->setToolTip("Copy selected PDF text (Ctrl+C)");
    connect(m_copy,&QAction::triggered,this,[this]{
        if(!m_canvas->copySelection()) statusBar()->showMessage("Select some text first.",2500);
    });

    m_highlight=new QAction("Highlight",this);
    m_highlight->setShortcut(QKeySequence("Ctrl+H"));
    m_highlight->setToolTip("Highlight the current text selection");
    connect(m_highlight,&QAction::triggered,this,[this]{
        if(!m_canvas->addHighlightFromSelection()) statusBar()->showMessage("Select text first.",2500);
    });

    m_cosInspect=new QAction("Inspect COS",this);
    m_cosInspect->setToolTip("Inspect the PDF object structure");
    connect(m_cosInspect,&QAction::triggered,this,&MainWindow::inspectCos);

    QMenu *fileMenu=menuBar()->addMenu("&File");
    fileMenu->addAction(m_open);
    fileMenu->addSeparator();
    QAction *exitAction=fileMenu->addAction("Exit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction,&QAction::triggered,this,&QWidget::close);

    QMenu *viewMenu=menuBar()->addMenu("&View");
    viewMenu->addAction("Zoom In",m_canvas,&PdfCanvas::zoomIn,QKeySequence::ZoomIn);
    viewMenu->addAction("Zoom Out",m_canvas,&PdfCanvas::zoomOut,QKeySequence::ZoomOut);
}

void MainWindow::buildToolbar()
{
    QToolBar *tb=addToolBar("Reader");
    tb->setObjectName("readerToolbar");
    tb->setMovable(false);
    tb->setFloatable(false);
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    tb->setIconSize(QSize(18,18));

    tb->addAction(m_open);
    if(auto *button=qobject_cast<QToolButton*>(tb->widgetForAction(m_open)))
        button->setProperty("openPdfButton",true);

    tb->addSeparator();

    QAction *prev=new QAction(style()->standardIcon(QStyle::SP_ArrowBack),"Previous",this);
    QAction *next=new QAction(style()->standardIcon(QStyle::SP_ArrowForward),"Next",this);
    prev->setShortcut(QKeySequence(Qt::Key_PageUp));
    next->setShortcut(QKeySequence(Qt::Key_PageDown));
    prev->setToolTip("Previous page (Page Up)");
    next->setToolTip("Next page (Page Down)");
    tb->addAction(prev);
    tb->addAction(next);
    connect(prev,&QAction::triggered,this,[this]{m_canvas->setCurrentPage(m_canvas->currentPage()-1);});
    connect(next,&QAction::triggered,this,[this]{m_canvas->setCurrentPage(m_canvas->currentPage()+1);});

    QLabel *pageLabel=new QLabel("  Page ",tb);
    tb->addWidget(pageLabel);
    m_pageSpin=new QSpinBox(tb);
    m_pageSpin->setRange(1,1);
    m_pageSpin->setFixedWidth(72);
    m_pageSpin->setAlignment(Qt::AlignCenter);
    m_pageSpin->setToolTip("Jump to page");
    tb->addWidget(m_pageSpin);
    m_pageTotal=new QLabel(" / 0  ",tb);
    tb->addWidget(m_pageTotal);
    connect(m_pageSpin,qOverload<int>(&QSpinBox::valueChanged),this,[this](int p){m_canvas->setCurrentPage(p-1);});

    tb->addSeparator();

    QAction *zout=tb->addAction("−");
    QAction *zin=tb->addAction("+");
    QAction *z100=tb->addAction("100%");
    zout->setToolTip("Zoom out");
    zin->setToolTip("Zoom in");
    z100->setToolTip("Reset zoom to 100%");
    connect(zout,&QAction::triggered,m_canvas,&PdfCanvas::zoomOut);
    connect(zin,&QAction::triggered,m_canvas,&PdfCanvas::zoomIn);
    connect(z100,&QAction::triggered,this,[this]{m_canvas->setZoom(1.0);updateZoomUi();});
    connect(zout,&QAction::triggered,this,&MainWindow::updateZoomUi);
    connect(zin,&QAction::triggered,this,&MainWindow::updateZoomUi);
    m_zoomLabel=new QLabel(" 100% ",tb);
    m_zoomLabel->setMinimumWidth(58);
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    tb->addWidget(m_zoomLabel);

    tb->addSeparator();

    m_modeCombo=new QComboBox(tb);
    m_modeCombo->addItems({"Single Page","Continuous","Facing Pages","Continuous Facing"});
    m_modeCombo->setCurrentIndex(0);
    m_modeCombo->setMinimumWidth(150);
    m_modeCombo->setToolTip("Choose page layout");
    tb->addWidget(m_modeCombo);
    connect(m_modeCombo,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{applyModeFromCombo();});

    tb->addSeparator();

    m_search=new QLineEdit(tb);
    m_search->setPlaceholderText("Search in PDF...");
    m_search->setClearButtonEnabled(true);
    m_search->setMinimumWidth(190);
    m_search->setMaximumWidth(280);
    m_search->setToolTip("Type text and press Enter");
    tb->addWidget(m_search);
    QAction *find=tb->addAction("Find Next");
    find->setToolTip("Find the next match");
    connect(find,&QAction::triggered,this,[this]{m_canvas->findNext(m_search->text());});
    connect(m_search,&QLineEdit::returnPressed,this,[this]{m_canvas->findNext(m_search->text());});

    tb->addSeparator();
    tb->addAction(m_copy);
    tb->addAction(m_highlight);
    tb->addAction(m_cosInspect);
}

void MainWindow::openPdf(const QString& filePath)
{
    m_document->close();
    m_currentFile=filePath;
    statusBar()->showMessage(QStringLiteral("Loading %1...").arg(QFileInfo(filePath).fileName()));

    const auto err=m_document->load(filePath);
    if(err!=QPdfDocument::Error::None){
        QMessageBox::critical(this,"Open PDF",QStringLiteral("Could not open PDF. Error code: %1").arg(static_cast<int>(err)));
        m_currentFile.clear();
        return;
    }

    if(m_document->status()==QPdfDocument::Status::Ready)
        finishOpenPdf();
}

void MainWindow::finishOpenPdf()
{
    if(m_currentFile.isEmpty() || m_document->status()!=QPdfDocument::Status::Ready)
        return;

    m_canvas->setDocument(m_document);
    m_canvas->setViewportWidth(m_scrollArea->viewport()->width());

    const int count=m_document->pageCount();
    { QSignalBlocker b(m_pageSpin); m_pageSpin->setRange(1,std::max(1,count)); m_pageSpin->setValue(1); }
    m_pageTotal->setText(QStringLiteral(" / %1  ").arg(count));
    setWindowTitle(QStringLiteral("%1  •  AstraPDF").arg(QFileInfo(m_currentFile).fileName()));
    statusBar()->showMessage(QStringLiteral("Opened %1  •  %2 pages").arg(QFileInfo(m_currentFile).fileName()).arg(count),5000);
}

void MainWindow::applyModeFromCombo()
{
    switch(m_modeCombo->currentIndex()){
    case 0:m_canvas->setViewMode(ViewMode::SinglePage);break;
    case 1:m_canvas->setViewMode(ViewMode::Continuous);break;
    case 2:m_canvas->setViewMode(ViewMode::FacingPages);break;
    case 3:m_canvas->setViewMode(ViewMode::ContinuousFacing);break;
    }
    statusBar()->showMessage(QStringLiteral("View: %1").arg(m_modeCombo->currentText()),1800);
}

void MainWindow::updatePageUi(int page)
{
    QSignalBlocker b(m_pageSpin);
    m_pageSpin->setValue(page+1);
}

void MainWindow::updateZoomUi()
{
    m_zoomLabel->setText(QStringLiteral(" %1% ").arg(int(m_canvas->zoom()*100.0+0.5)));
}

void MainWindow::inspectCos()
{
    if(m_currentFile.isEmpty()){
        statusBar()->showMessage("Open a PDF first.",2500);
        return;
    }
    CosSummary s=CosInspector::inspect(m_currentFile);
    auto *viewer=new QTextEdit;
    viewer->setReadOnly(true);
    viewer->setPlainText(s.humanReadable);
    auto *dialog=new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("PDF COS / Physical Structure");
    auto *layout=new QVBoxLayout(dialog);
    layout->addWidget(viewer);
    dialog->resize(800,600);
    dialog->show();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if(m_scrollArea&&m_canvas)
        m_canvas->setViewportWidth(m_scrollArea->viewport()->width());
}
