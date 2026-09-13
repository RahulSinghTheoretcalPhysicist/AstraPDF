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
#include <QMessageBox>
#include <QPdfDocument>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStatusBar>
#include <QTextEdit>
#include <QToolBar>
#include <QVBoxLayout>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent),
    m_document(new QPdfDocument(this)),m_scrollArea(new QScrollArea(this)),m_canvas(new PdfCanvas)
{
    buildUi(); buildActions(); buildToolbar(); m_canvas->setDocument(m_document);
    connect(m_canvas,&PdfCanvas::currentPageChanged,this,&MainWindow::updatePageUi);
    connect(m_canvas,&PdfCanvas::requestEnsureVisible,this,[this](const QRectF& r){
        m_scrollArea->ensureVisible(int(r.center().x()),int(r.center().y()),
                                   int(std::max<qreal>(40.0,r.width()/2.0)),
                                   int(std::max<qreal>(40.0,r.height()/2.0)));
    });
    connect(m_canvas,&PdfCanvas::statusMessage,this,[this](const QString& s){statusBar()->showMessage(s,4000);});
    resize(1280,860);
}

void MainWindow::buildUi()
{
    m_scrollArea->setWidget(m_canvas); m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setAlignment(Qt::AlignLeft|Qt::AlignTop);
    setCentralWidget(m_scrollArea); setWindowTitle("AstraPDF"); statusBar()->showMessage("Ready");
}

void MainWindow::buildActions()
{
    m_open=new QAction("Open",this); m_open->setShortcut(QKeySequence::Open);
    connect(m_open,&QAction::triggered,this,[this]{
        QString p=QFileDialog::getOpenFileName(this,"Open PDF",{},"PDF documents (*.pdf)");
        if(!p.isEmpty())openPdf(p);
    });
    m_copy=new QAction("Copy",this); m_copy->setShortcut(QKeySequence::Copy);
    connect(m_copy,&QAction::triggered,this,[this]{if(!m_canvas->copySelection())statusBar()->showMessage("No text selected.",2500);});
    m_highlight=new QAction("Highlight",this); m_highlight->setShortcut(QKeySequence("Ctrl+H"));
    connect(m_highlight,&QAction::triggered,this,[this]{if(!m_canvas->addHighlightFromSelection())statusBar()->showMessage("Select text first.",2500);});
    m_cosInspect=new QAction("Inspect COS",this);
    connect(m_cosInspect,&QAction::triggered,this,&MainWindow::inspectCos);
}

void MainWindow::buildToolbar()
{
    QToolBar *tb=addToolBar("Reader"); tb->setMovable(false); tb->addAction(m_open); tb->addSeparator();
    QAction *prev=tb->addAction("◀"),*next=tb->addAction("▶");
    connect(prev,&QAction::triggered,this,[this]{m_canvas->setCurrentPage(m_canvas->currentPage()-1);});
    connect(next,&QAction::triggered,this,[this]{m_canvas->setCurrentPage(m_canvas->currentPage()+1);});

    m_pageSpin=new QSpinBox(tb); m_pageSpin->setRange(1,1); m_pageSpin->setFixedWidth(72); tb->addWidget(m_pageSpin);
    m_pageTotal=new QLabel("/ 0",tb); tb->addWidget(m_pageTotal);
    connect(m_pageSpin,qOverload<int>(&QSpinBox::valueChanged),this,[this](int p){m_canvas->setCurrentPage(p-1);});

    tb->addSeparator();
    QAction *zout=tb->addAction("−"),*zin=tb->addAction("+"),*z100=tb->addAction("100%");
    connect(zout,&QAction::triggered,m_canvas,&PdfCanvas::zoomOut);
    connect(zin,&QAction::triggered,m_canvas,&PdfCanvas::zoomIn);
    connect(z100,&QAction::triggered,this,[this]{m_canvas->setZoom(1.0);updateZoomUi();});
    connect(zout,&QAction::triggered,this,&MainWindow::updateZoomUi);
    connect(zin,&QAction::triggered,this,&MainWindow::updateZoomUi);
    m_zoomLabel=new QLabel("100%",tb); m_zoomLabel->setMinimumWidth(52); tb->addWidget(m_zoomLabel);

    tb->addSeparator();
    m_modeCombo=new QComboBox(tb);
    m_modeCombo->addItems({"Single Page","Continuous","Facing Pages","Continuous Facing"});
    m_modeCombo->setCurrentIndex(1); tb->addWidget(m_modeCombo);
    connect(m_modeCombo,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{applyModeFromCombo();});

    tb->addSeparator();
    m_search=new QLineEdit(tb); m_search->setPlaceholderText("Search text..."); m_search->setClearButtonEnabled(true); m_search->setFixedWidth(220); tb->addWidget(m_search);
    QAction *find=tb->addAction("Find Next");
    connect(find,&QAction::triggered,this,[this]{m_canvas->findNext(m_search->text());});
    connect(m_search,&QLineEdit::returnPressed,this,[this]{m_canvas->findNext(m_search->text());});

    tb->addSeparator(); tb->addAction(m_copy); tb->addAction(m_highlight); tb->addAction(m_cosInspect);
}

void MainWindow::openPdf(const QString& filePath)
{
    m_document->close(); m_currentFile.clear();
    auto err=m_document->load(filePath);
    if(err!=QPdfDocument::Error::None){
        QMessageBox::critical(this,"Open PDF",QStringLiteral("Could not open PDF. Error code: %1").arg(static_cast<int>(err))); return;
    }
    m_currentFile=filePath; m_canvas->setDocument(m_document); m_canvas->setViewportWidth(m_scrollArea->viewport()->width());
    const int count=m_document->pageCount();
    { QSignalBlocker b(m_pageSpin); m_pageSpin->setRange(1,std::max(1,count)); m_pageSpin->setValue(1); }
    m_pageTotal->setText(QStringLiteral("/ %1").arg(count));
    setWindowTitle(QStringLiteral("%1 - AstraPDF").arg(QFileInfo(filePath).fileName()));
    statusBar()->showMessage(QStringLiteral("Opened %1 pages.").arg(count),3500);
}

void MainWindow::applyModeFromCombo()
{
    switch(m_modeCombo->currentIndex()){
    case 0:m_canvas->setViewMode(ViewMode::SinglePage);break;
    case 1:m_canvas->setViewMode(ViewMode::Continuous);break;
    case 2:m_canvas->setViewMode(ViewMode::FacingPages);break;
    case 3:m_canvas->setViewMode(ViewMode::ContinuousFacing);break;
    }
}

void MainWindow::updatePageUi(int page){QSignalBlocker b(m_pageSpin);m_pageSpin->setValue(page+1);}
void MainWindow::updateZoomUi(){m_zoomLabel->setText(QStringLiteral("%1%").arg(int(m_canvas->zoom()*100.0+0.5)));}

void MainWindow::inspectCos()
{
    if(m_currentFile.isEmpty()){statusBar()->showMessage("Open a PDF first.",2500);return;}
    CosSummary s=CosInspector::inspect(m_currentFile);
    auto *viewer=new QTextEdit; viewer->setReadOnly(true); viewer->setPlainText(s.humanReadable);
    auto *dialog=new QDialog(this); dialog->setAttribute(Qt::WA_DeleteOnClose); dialog->setWindowTitle("PDF COS / Physical Structure");
    auto *layout=new QVBoxLayout(dialog); layout->addWidget(viewer); dialog->resize(800,600); dialog->show();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if(m_scrollArea&&m_canvas)m_canvas->setViewportWidth(m_scrollArea->viewport()->width());
}
