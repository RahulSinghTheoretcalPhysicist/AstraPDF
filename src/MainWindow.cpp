#include "MainWindow.h"
#include "PdfCanvas.h"
#include "CosInspector.h"
#include <QAction>
#include <QColor>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialog>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHash>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPdfDocument>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QTextBrowser>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <algorithm>

namespace {
QString normalizedWord(QString word)
{
    word=word.trimmed().toLower();
    word.remove(QRegularExpression(QStringLiteral("^[^a-zA-Z]+|[^a-zA-Z]+$")));
    return word;
}

QString definitionFor(const QString& key)
{
    static const QHash<QString,QString> dictionary={
        {"acceleration","The rate at which velocity changes with time."},
        {"action","In physics, a quantity obtained by integrating the Lagrangian over time; physical paths make the action stationary."},
        {"algorithm","A finite sequence of well-defined steps used to solve a problem or perform a computation."},
        {"atom","The smallest unit of an element that retains that element's chemical properties."},
        {"boundary","A limit or edge that separates one region, set, or physical domain from another."},
        {"calculus","The branch of mathematics concerned with change, accumulation, derivatives, and integrals."},
        {"coordinate","A number used to specify a position relative to a chosen reference system."},
        {"curvature","A measure of how much a geometric object or spacetime differs locally from being flat."},
        {"derivative","A measure of how rapidly one quantity changes with respect to another."},
        {"dimension","An independent direction or coordinate needed to specify a point in a space."},
        {"energy","A conserved physical quantity associated with the capacity of a system to produce change."},
        {"entropy","A measure related to the number of microscopic configurations compatible with a macroscopic state."},
        {"equation","A mathematical statement asserting that two expressions are equal."},
        {"field","A quantity defined at each point of space or spacetime, such as temperature, electric field, or a quantum field."},
        {"force","An interaction that can change an object's momentum."},
        {"frequency","The number of repeated cycles or oscillations per unit time."},
        {"function","A rule that assigns each allowed input exactly one output."},
        {"geometry","The study of shapes, distances, angles, spaces, and their structure."},
        {"gravity","The interaction associated with mass-energy; in general relativity it is described by spacetime curvature."},
        {"integral","A mathematical operation that accumulates infinitesimal contributions over an interval, region, or domain."},
        {"lagrangian","A function that encodes the dynamics of a system, commonly kinetic energy minus potential energy in classical mechanics."},
        {"manifold","A space that can be curved globally but looks locally like ordinary Euclidean space."},
        {"mass","A physical quantity measuring inertia and contributing to gravitation."},
        {"matrix","A rectangular array of numbers or mathematical objects used to represent linear transformations and related structures."},
        {"momentum","A conserved quantity associated with motion; classically it is mass times velocity."},
        {"operator","A mathematical object that acts on another object, often transforming a function, vector, or quantum state."},
        {"particle","An idealized localized physical entity; in quantum theory, particle behavior arises from quantum fields and states."},
        {"probability","A numerical measure, usually from 0 to 1, representing how likely an event is."},
        {"quantum","A discrete unit or a term referring to the physical framework governing microscopic systems."},
        {"relativity","The framework describing how measurements of space, time, motion, and gravity depend on observers and spacetime geometry."},
        {"scalar","A quantity described by a single value that is unchanged by coordinate-basis rotations of the relevant type."},
        {"spacetime","The four-dimensional structure combining three spatial dimensions with time."},
        {"symmetry","A transformation that leaves specified properties of a system unchanged."},
        {"tensor","A multilinear geometric object whose components transform according to precise coordinate-transformation rules."},
        {"time","A coordinate used to order events and describe change."},
        {"topology","The study of properties of spaces preserved under continuous deformation."},
        {"vector","An object with magnitude and direction, or more generally an element of a vector space."},
        {"velocity","The rate of change of position with time, including direction."},
        {"wave","A disturbance or oscillatory pattern that propagates through a medium or field."},
        {"wavelength","The spatial distance between successive equivalent points of a periodic wave."}
    };
    return dictionary.value(key);
}

QString plainWikipediaSnippet(QString snippet)
{
    snippet.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
    return snippet;
}
}

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent),
    m_document(new QPdfDocument(this)),m_scrollArea(new QScrollArea(this)),m_canvas(new PdfCanvas),
    m_network(new QNetworkAccessManager(this))
{
    buildUi();
    buildActions();
    buildToolbar();
    buildDictionaryDock();
    m_canvas->setDocument(m_document);

    m_neonTimer=new QTimer(this);
    m_neonTimer->setInterval(120);
    connect(m_neonTimer,&QTimer::timeout,this,&MainWindow::advanceNeon);
    m_neonTimer->start();

    connect(m_canvas,&PdfCanvas::currentPageChanged,this,&MainWindow::updatePageUi);
    connect(m_canvas,&PdfCanvas::requestEnsureVisible,this,[this](const QRectF& r){
        m_scrollArea->ensureVisible(int(r.center().x()),int(r.center().y()),
                                   int(std::max<qreal>(40.0,r.width()/2.0)),
                                   int(std::max<qreal>(40.0,r.height()/2.0)));
    });
    connect(m_canvas,&PdfCanvas::statusMessage,this,[this](const QString& s){ statusBar()->showMessage(s,4000); });
    connect(m_canvas,&PdfCanvas::selectionChanged,this,[this](const QString& text){
        const QString trimmed=text.trimmed();
        if(auto *webInput=findChild<QLineEdit*>("webSearchInput")){
            if(!trimmed.isEmpty()) webInput->setText(trimmed.left(500));
        }
        if(trimmed.isEmpty() || trimmed.contains(QRegularExpression(QStringLiteral("\\s")))) return;
        if(m_dictionaryInput) m_dictionaryInput->setText(trimmed);
        if(m_dictionaryDock && m_dictionaryDock->isVisible()) lookupDictionaryWord(trimmed);
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
    applyTheme();
    statusBar()->showMessage("Ready  •  Open a PDF with the blue button or press Ctrl+O");
}

void MainWindow::applyTheme()
{
    const QColor accent=m_neonEnabled ? QColor::fromHsv(m_neonHue,210,255) : QColor("#2f6feb");
    const QColor accentSoft=accent.darker(145);
    const QString a=accent.name();
    const QString a2=accentSoft.name();
    setStyleSheet(QStringLiteral(R"(
        QMainWindow#mainWindow { background:#12151a; }
        QMenuBar { background:#191e25; color:#e8edf3; border-bottom:1px solid %1; padding:3px 6px; }
        QMenuBar::item { padding:6px 10px; border-radius:5px; }
        QMenuBar::item:selected { background:%2; }
        QMenu { background:#20252d; color:#edf1f5; border:1px solid %1; padding:6px; }
        QMenu::item { padding:7px 28px 7px 10px; border-radius:5px; }
        QMenu::item:selected { background:%2; }
        QToolBar { background:#191e25; border:none; border-bottom:1px solid %1; spacing:6px; padding:8px 10px; }
        QToolBar::separator { background:#343a43; width:1px; margin:6px 5px; }
        QToolButton, QPushButton { color:#e7ecf2; background:#272d36; border:1px solid #3b4654; border-radius:8px; padding:7px 10px; min-height:20px; }
        QToolButton:hover, QPushButton:hover { background:%2; border-color:%1; }
        QToolButton:pressed, QPushButton:pressed { background:#101318; }
        QToolButton[openPdfButton="true"] { background:%1; border-color:%1; color:#071014; font-weight:700; padding-left:14px; padding-right:14px; }
        QSpinBox, QComboBox, QLineEdit { background:#101419; color:#edf1f5; border:1px solid #39414d; border-radius:7px; padding:6px 8px; min-height:22px; selection-background-color:%1; }
        QSpinBox:hover, QComboBox:hover, QLineEdit:hover { border-color:%1; }
        QSpinBox:focus, QComboBox:focus, QLineEdit:focus { border:1px solid %1; }
        QLabel { color:#cbd2dc; }
        QScrollArea#documentArea { background:#0d1116; }
        QStatusBar { background:#191e25; color:#aeb7c4; border-top:1px solid %1; }
        QDockWidget { color:#edf1f5; font-weight:600; }
        QDockWidget::title { background:#191e25; border:1px solid %1; padding:7px 10px; text-align:left; }
        QDockWidget QWidget { background:#151a20; color:#edf1f5; }
        QTextBrowser { background:#0f1318; color:#dce3ec; border:1px solid %1; border-radius:8px; padding:8px; }
    )").arg(a,a2));
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
    connect(m_copy,&QAction::triggered,this,[this]{ if(!m_canvas->copySelection()) statusBar()->showMessage("Select some text first.",2500); });

    m_highlight=new QAction("Highlight",this);
    m_highlight->setShortcut(QKeySequence("Ctrl+H"));
    connect(m_highlight,&QAction::triggered,this,[this]{ if(!m_canvas->addHighlightFromSelection()) statusBar()->showMessage("Select text first.",2500); });

    m_cosInspect=new QAction("Inspect COS",this);
    connect(m_cosInspect,&QAction::triggered,this,&MainWindow::inspectCos);

    m_neonAction=new QAction("Neon Color Cycle",this);
    m_neonAction->setCheckable(true);
    m_neonAction->setChecked(true);
    connect(m_neonAction,&QAction::toggled,this,&MainWindow::setNeonEnabled);

    m_dictionaryAction=new QAction("Dictionary",this);
    m_dictionaryAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(m_dictionaryAction,&QAction::triggered,this,[this]{
        if(m_dictionaryDock){ m_dictionaryDock->show(); m_dictionaryDock->raise(); }
        if(m_dictionaryInput) m_dictionaryInput->setFocus();
    });

    QMenu *fileMenu=menuBar()->addMenu("&File");
    fileMenu->addAction(m_open);
    fileMenu->addSeparator();
    QAction *exitAction=fileMenu->addAction("Exit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction,&QAction::triggered,this,&QWidget::close);

    QMenu *viewMenu=menuBar()->addMenu("&View");
    viewMenu->addAction("Zoom In",m_canvas,&PdfCanvas::zoomIn,QKeySequence::ZoomIn);
    viewMenu->addAction("Zoom Out",m_canvas,&PdfCanvas::zoomOut,QKeySequence::ZoomOut);
    viewMenu->addSeparator();
    viewMenu->addAction(m_neonAction);
    viewMenu->addAction(m_dictionaryAction);
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
    if(auto *button=qobject_cast<QToolButton*>(tb->widgetForAction(m_open))) button->setProperty("openPdfButton",true);
    tb->addSeparator();

    QAction *prev=new QAction(style()->standardIcon(QStyle::SP_ArrowBack),"Previous",this);
    QAction *next=new QAction(style()->standardIcon(QStyle::SP_ArrowForward),"Next",this);
    prev->setShortcut(QKeySequence(Qt::Key_PageUp));
    next->setShortcut(QKeySequence(Qt::Key_PageDown));
    tb->addAction(prev); tb->addAction(next);
    connect(prev,&QAction::triggered,this,[this]{m_canvas->setCurrentPage(m_canvas->currentPage()-1);});
    connect(next,&QAction::triggered,this,[this]{m_canvas->setCurrentPage(m_canvas->currentPage()+1);});

    tb->addWidget(new QLabel("  Page ",tb));
    m_pageSpin=new QSpinBox(tb);
    m_pageSpin->setRange(1,1); m_pageSpin->setFixedWidth(72); m_pageSpin->setAlignment(Qt::AlignCenter);
    tb->addWidget(m_pageSpin);
    m_pageTotal=new QLabel(" / 0  ",tb); tb->addWidget(m_pageTotal);
    connect(m_pageSpin,qOverload<int>(&QSpinBox::valueChanged),this,[this](int p){m_canvas->setCurrentPage(p-1);});

    tb->addSeparator();
    QAction *zout=tb->addAction("−"),*zin=tb->addAction("+"),*z100=tb->addAction("100%");
    connect(zout,&QAction::triggered,m_canvas,&PdfCanvas::zoomOut);
    connect(zin,&QAction::triggered,m_canvas,&PdfCanvas::zoomIn);
    connect(z100,&QAction::triggered,this,[this]{m_canvas->setZoom(1.0);updateZoomUi();});
    connect(zout,&QAction::triggered,this,&MainWindow::updateZoomUi);
    connect(zin,&QAction::triggered,this,&MainWindow::updateZoomUi);
    m_zoomLabel=new QLabel(" 100% ",tb); m_zoomLabel->setMinimumWidth(58); m_zoomLabel->setAlignment(Qt::AlignCenter); tb->addWidget(m_zoomLabel);

    tb->addSeparator();
    m_modeCombo=new QComboBox(tb);
    m_modeCombo->addItems({"Single Page","Continuous","Facing Pages","Continuous Facing"});
    m_modeCombo->setCurrentIndex(0); m_modeCombo->setMinimumWidth(150); tb->addWidget(m_modeCombo);
    connect(m_modeCombo,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{applyModeFromCombo();});

    tb->addSeparator();
    m_search=new QLineEdit(tb);
    m_search->setPlaceholderText("Search in PDF..."); m_search->setClearButtonEnabled(true); m_search->setMinimumWidth(190); m_search->setMaximumWidth(280);
    tb->addWidget(m_search);
    QAction *find=tb->addAction("Find Next");
    connect(find,&QAction::triggered,this,[this]{m_canvas->findNext(m_search->text());});
    connect(m_search,&QLineEdit::returnPressed,this,[this]{m_canvas->findNext(m_search->text());});

    tb->addSeparator();
    tb->addAction(m_dictionaryAction);

    QAction *webSearchAction=new QAction("Search",this);
    webSearchAction->setShortcut(QKeySequence("Ctrl+Shift+F"));
    webSearchAction->setToolTip("Search DuckDuckGo / Wikipedia (Ctrl+Shift+F)");
    tb->addAction(webSearchAction);

    auto *webDock=new QDockWidget("Web Search",this);
    webDock->setObjectName("webSearchDock");
    webDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    auto *webPanel=new QWidget(webDock);
    auto *webLayout=new QVBoxLayout(webPanel);
    webLayout->setContentsMargins(10,10,10,10);
    webLayout->setSpacing(8);
    auto *webHint=new QLabel("Search the web without leaving your PDF workflow. Selected PDF text is copied here automatically.",webPanel);
    webHint->setWordWrap(true);
    webLayout->addWidget(webHint);
    auto *provider=new QComboBox(webPanel);
    provider->setObjectName("webSearchProvider");
    provider->addItems({"DuckDuckGo + Wikipedia","DuckDuckGo","Wikipedia"});
    webLayout->addWidget(provider);
    auto *webRow=new QHBoxLayout;
    auto *webInput=new QLineEdit(webPanel);
    webInput->setObjectName("webSearchInput");
    webInput->setPlaceholderText("Search query...");
    webInput->setClearButtonEnabled(true);
    auto *webGo=new QPushButton("Search",webPanel);
    webRow->addWidget(webInput,1);
    webRow->addWidget(webGo);
    webLayout->addLayout(webRow);
    auto *webResult=new QTextBrowser(webPanel);
    webResult->setObjectName("webSearchResult");
    webResult->setOpenLinks(false);
    webResult->setOpenExternalLinks(false);
    webResult->setHtml("<h3>Web Search ready</h3><p>Use DuckDuckGo, Wikipedia, or both. No paid API is required.</p>");
    webLayout->addWidget(webResult,1);
    auto *webNote=new QLabel("Free search • no API key • result links open in your normal browser",webPanel);
    webNote->setWordWrap(true);
    webLayout->addWidget(webNote);
    webDock->setWidget(webPanel);
    addDockWidget(Qt::RightDockWidgetArea,webDock);
    webDock->hide();

    connect(webResult,&QTextBrowser::anchorClicked,this,[](const QUrl& url){ QDesktopServices::openUrl(url); });

    auto runWebSearch=[this,webInput,provider,webResult]{
        const QString query=webInput->text().trimmed();
        if(query.isEmpty()){
            webResult->setHtml("<h3>Web Search</h3><p>Enter something to search.</p>");
            return;
        }
        webInput->setProperty("currentQuery",query);
        webResult->setHtml(QStringLiteral("<h2>%1</h2><p><i>Searching…</i></p>").arg(query.toHtmlEscaped()));
        const int mode=provider->currentIndex();

        if(mode==0 || mode==1){
            QUrl url("https://api.duckduckgo.com/");
            QUrlQuery params;
            params.addQueryItem("q",query);
            params.addQueryItem("format","json");
            params.addQueryItem("no_html","1");
            params.addQueryItem("no_redirect","1");
            params.addQueryItem("skip_disambig","1");
            url.setQuery(params);
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::UserAgentHeader,"AstraPDF/0.2");
            request.setTransferTimeout(8000);
            QNetworkReply *reply=m_network->get(request);
            connect(reply,&QNetworkReply::finished,this,[reply,webInput,webResult,query]{
                const QByteArray data=reply->readAll();
                const bool ok=reply->error()==QNetworkReply::NoError;
                reply->deleteLater();
                if(webInput->property("currentQuery").toString()!=query) return;
                QString html="<hr><h3>DuckDuckGo</h3>";
                if(!ok){
                    webResult->append(html+"<p>DuckDuckGo is unavailable right now.</p>");
                    return;
                }
                const QJsonDocument doc=QJsonDocument::fromJson(data);
                if(!doc.isObject()){
                    webResult->append(html+"<p>No DuckDuckGo result.</p>");
                    return;
                }
                const QJsonObject obj=doc.object();
                const QString heading=obj.value("Heading").toString();
                const QString abstractText=obj.value("AbstractText").toString();
                const QString abstractUrl=obj.value("AbstractURL").toString();
                if(!heading.isEmpty()) html += QStringLiteral("<h4>%1</h4>").arg(heading.toHtmlEscaped());
                if(!abstractText.isEmpty()) html += QStringLiteral("<p>%1</p>").arg(abstractText.toHtmlEscaped());
                if(!abstractUrl.isEmpty()) html += QStringLiteral("<p><a href=\"%1\">Open source</a></p>").arg(abstractUrl.toHtmlEscaped());
                int shown=0;
                const QJsonArray topics=obj.value("RelatedTopics").toArray();
                for(const QJsonValue& v:topics){
                    if(shown>=8) break;
                    QJsonObject item=v.toObject();
                    if(item.contains("Topics")){
                        const QJsonArray nested=item.value("Topics").toArray();
                        for(const QJsonValue& nv:nested){
                            if(shown>=8) break;
                            const QJsonObject n=nv.toObject();
                            const QString text=n.value("Text").toString();
                            const QString link=n.value("FirstURL").toString();
                            if(text.isEmpty()) continue;
                            html += link.isEmpty() ? QStringLiteral("<p>• %1</p>").arg(text.toHtmlEscaped()) : QStringLiteral("<p>• <a href=\"%1\">%2</a></p>").arg(link.toHtmlEscaped(),text.toHtmlEscaped());
                            ++shown;
                        }
                    }else{
                        const QString text=item.value("Text").toString();
                        const QString link=item.value("FirstURL").toString();
                        if(text.isEmpty()) continue;
                        html += link.isEmpty() ? QStringLiteral("<p>• %1</p>").arg(text.toHtmlEscaped()) : QStringLiteral("<p>• <a href=\"%1\">%2</a></p>").arg(link.toHtmlEscaped(),text.toHtmlEscaped());
                        ++shown;
                    }
                }
                if(abstractText.isEmpty() && shown==0) html += "<p>No instant answer found. Try Wikipedia or open a web result from your browser.</p>";
                webResult->append(html);
            });
        }

        if(mode==0 || mode==2){
            QUrl url("https://en.wikipedia.org/w/api.php");
            QUrlQuery params;
            params.addQueryItem("action","query");
            params.addQueryItem("list","search");
            params.addQueryItem("srsearch",query);
            params.addQueryItem("format","json");
            params.addQueryItem("utf8","1");
            params.addQueryItem("srlimit","8");
            url.setQuery(params);
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::UserAgentHeader,"AstraPDF/0.2");
            request.setTransferTimeout(8000);
            QNetworkReply *reply=m_network->get(request);
            connect(reply,&QNetworkReply::finished,this,[reply,webInput,webResult,query]{
                const QByteArray data=reply->readAll();
                const bool ok=reply->error()==QNetworkReply::NoError;
                reply->deleteLater();
                if(webInput->property("currentQuery").toString()!=query) return;
                QString html="<hr><h3>Wikipedia</h3>";
                if(!ok){
                    webResult->append(html+"<p>Wikipedia is unavailable right now.</p>");
                    return;
                }
                const QJsonDocument doc=QJsonDocument::fromJson(data);
                const QJsonArray results=doc.object().value("query").toObject().value("search").toArray();
                if(results.isEmpty()){
                    webResult->append(html+"<p>No Wikipedia results found.</p>");
                    return;
                }
                for(const QJsonValue& value:results){
                    const QJsonObject item=value.toObject();
                    QString title=item.value("title").toString();
                    const QString snippet=plainWikipediaSnippet(item.value("snippet").toString());
                    QUrl article("https://en.wikipedia.org/wiki/"+QString::fromUtf8(QUrl::toPercentEncoding(title.replace(' ','_'))));
                    html += QStringLiteral("<p><b><a href=\"%1\">%2</a></b><br>%3</p>").arg(article.toString().toHtmlEscaped(),title.toHtmlEscaped(),snippet.toHtmlEscaped());
                }
                webResult->append(html);
            });
        }
        statusBar()->showMessage(QStringLiteral("Searching web: %1").arg(query),1800);
    };

    connect(webSearchAction,&QAction::triggered,this,[webDock,webInput]{ webDock->show(); webDock->raise(); webInput->setFocus(); });
    connect(webGo,&QPushButton::clicked,this,runWebSearch);
    connect(webInput,&QLineEdit::returnPressed,this,runWebSearch);

    tb->addAction(m_copy);
    tb->addAction(m_highlight);
    tb->addAction(m_cosInspect);
}

void MainWindow::buildDictionaryDock()
{
    m_dictionaryDock=new QDockWidget("Dictionary",this);
    m_dictionaryDock->setObjectName("dictionaryDock");
    m_dictionaryDock->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);

    auto *panel=new QWidget(m_dictionaryDock);
    auto *layout=new QVBoxLayout(panel);
    layout->setContentsMargins(10,10,10,10);
    layout->setSpacing(8);

    auto *hint=new QLabel("Type a word, or select one word in the PDF.",panel);
    hint->setWordWrap(true);
    layout->addWidget(hint);

    m_dictionaryMode=new QComboBox(panel);
    m_dictionaryMode->addItems({"Auto (Offline + Online)","Offline only","Online only"});
    m_dictionaryMode->setToolTip("Auto shows offline instantly and uses the online dictionary when internet is available.");
    layout->addWidget(m_dictionaryMode);

    auto *row=new QHBoxLayout;
    m_dictionaryInput=new QLineEdit(panel);
    m_dictionaryInput->setPlaceholderText("Dictionary word...");
    m_dictionaryInput->setClearButtonEnabled(true);
    auto *lookup=new QPushButton("Look up",panel);
    row->addWidget(m_dictionaryInput,1);
    row->addWidget(lookup);
    layout->addLayout(row);

    m_dictionaryResult=new QTextBrowser(panel);
    m_dictionaryResult->setOpenExternalLinks(false);
    m_dictionaryResult->setHtml("<h3>Dictionary ready</h3><p><b>Auto</b> mode works offline first and adds an online definition when internet is available.</p>");
    layout->addWidget(m_dictionaryResult,1);

    auto *note=new QLabel("Auto • Offline • Online   |   Online lookup stays inside AstraPDF",panel);
    note->setWordWrap(true);
    layout->addWidget(note);

    m_dictionaryDock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea,m_dictionaryDock);
    m_dictionaryDock->hide();

    connect(lookup,&QPushButton::clicked,this,[this]{ lookupDictionaryWord(m_dictionaryInput->text()); });
    connect(m_dictionaryInput,&QLineEdit::returnPressed,this,[this]{ lookupDictionaryWord(m_dictionaryInput->text()); });
    connect(m_dictionaryMode,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{
        if(m_dictionaryInput && !m_dictionaryInput->text().trimmed().isEmpty()) lookupDictionaryWord(m_dictionaryInput->text());
    });
}

void MainWindow::setNeonEnabled(bool enabled)
{
    m_neonEnabled=enabled;
    if(m_neonTimer){ if(enabled) m_neonTimer->start(); else m_neonTimer->stop(); }
    applyTheme();
    statusBar()->showMessage(enabled ? "Neon color cycle: ON" : "Neon color cycle: OFF",1800);
}

void MainWindow::advanceNeon()
{
    if(!m_neonEnabled) return;
    m_neonHue=(m_neonHue+2)%360;
    applyTheme();
}

void MainWindow::showOfflineDictionaryWord(const QString& word, const QString& definition, bool waitingForOnline)
{
    QString html=QStringLiteral("<h2>%1</h2>").arg(word.toHtmlEscaped());
    if(definition.isEmpty())
        html += "<p><b>Offline:</b> No local definition is stored for this word.</p>";
    else
        html += QStringLiteral("<p><b>Offline:</b> %1</p>").arg(definition.toHtmlEscaped());
    if(waitingForOnline)
        html += "<hr><p><i>Checking online dictionary…</i></p>";
    m_dictionaryResult->setHtml(html);
}

void MainWindow::lookupDictionaryWord(const QString& rawWord)
{
    const QString key=normalizedWord(rawWord);
    if(key.isEmpty()){
        m_dictionaryResult->setHtml("<h3>Dictionary</h3><p>Enter a word first.</p>");
        return;
    }

    if(m_dictionaryInput && m_dictionaryInput->text()!=key) m_dictionaryInput->setText(key);
    const QString offlineDefinition=definitionFor(key);
    const int mode=m_dictionaryMode ? m_dictionaryMode->currentIndex() : 0;

    if(mode==1){
        showOfflineDictionaryWord(key,offlineDefinition,false);
        statusBar()->showMessage("Dictionary: offline lookup",1800);
        return;
    }

    if(mode==0) showOfflineDictionaryWord(key,offlineDefinition,true);
    else m_dictionaryResult->setHtml(QStringLiteral("<h2>%1</h2><p><i>Checking online dictionary…</i></p>").arg(key.toHtmlEscaped()));

    lookupOnlineDictionaryWord(key,offlineDefinition);
}

void MainWindow::lookupOnlineDictionaryWord(const QString& word, const QString& offlineDefinition)
{
    const QString encoded=QString::fromUtf8(QUrl::toPercentEncoding(word));
    QNetworkRequest request(QUrl(QStringLiteral("https://api.dictionaryapi.dev/api/v2/entries/en/%1").arg(encoded)));
    request.setHeader(QNetworkRequest::UserAgentHeader,"AstraPDF/0.2");
    request.setTransferTimeout(8000);
    QNetworkReply *reply=m_network->get(request);

    connect(reply,&QNetworkReply::finished,this,[this,reply,word,offlineDefinition]{
        const int mode=m_dictionaryMode ? m_dictionaryMode->currentIndex() : 0;
        const QByteArray data=reply->readAll();
        const bool networkOk=reply->error()==QNetworkReply::NoError;
        reply->deleteLater();

        if(!networkOk){
            if(mode==2){
                m_dictionaryResult->setHtml(QStringLiteral("<h2>%1</h2><p><b>Online dictionary unavailable.</b></p><p>Check the internet connection and try again.</p>").arg(word.toHtmlEscaped()));
            }else{
                showOfflineDictionaryWord(word,offlineDefinition,false);
                if(!offlineDefinition.isEmpty()) m_dictionaryResult->append("<p><i>Online lookup unavailable. Showing offline definition.</i></p>");
                else m_dictionaryResult->append("<p><i>Online lookup unavailable and this word is not in the local starter dictionary.</i></p>");
            }
            statusBar()->showMessage("Online dictionary unavailable",2500);
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document=QJsonDocument::fromJson(data,&parseError);
        if(parseError.error!=QJsonParseError::NoError || !document.isArray() || document.array().isEmpty()){
            if(mode==2)
                m_dictionaryResult->setHtml(QStringLiteral("<h2>%1</h2><p>No online definition found.</p>").arg(word.toHtmlEscaped()));
            else{
                showOfflineDictionaryWord(word,offlineDefinition,false);
                m_dictionaryResult->append("<p><i>No additional online definition found.</i></p>");
            }
            return;
        }

        const QJsonObject entry=document.array().first().toObject();
        QString html=QStringLiteral("<h2>%1</h2>").arg(word.toHtmlEscaped());
        if(mode==0 && !offlineDefinition.isEmpty())
            html += QStringLiteral("<p><b>Offline:</b> %1</p><hr>").arg(offlineDefinition.toHtmlEscaped());

        const QString phonetic=entry.value("phonetic").toString();
        if(!phonetic.isEmpty()) html += QStringLiteral("<p><b>Pronunciation:</b> %1</p>").arg(phonetic.toHtmlEscaped());
        html += "<p><b>Online:</b></p>";

        const QJsonArray meanings=entry.value("meanings").toArray();
        int meaningCount=0;
        for(const QJsonValue& meaningValue:meanings){
            if(meaningCount>=4) break;
            const QJsonObject meaning=meaningValue.toObject();
            const QString part=meaning.value("partOfSpeech").toString();
            if(!part.isEmpty()) html += QStringLiteral("<h4>%1</h4>").arg(part.toHtmlEscaped());
            const QJsonArray definitions=meaning.value("definitions").toArray();
            int defCount=0;
            for(const QJsonValue& defValue:definitions){
                if(defCount>=2) break;
                const QJsonObject def=defValue.toObject();
                const QString text=def.value("definition").toString();
                if(text.isEmpty()) continue;
                html += QStringLiteral("<p>• %1</p>").arg(text.toHtmlEscaped());
                const QString example=def.value("example").toString();
                if(!example.isEmpty()) html += QStringLiteral("<p><i>Example: %1</i></p>").arg(example.toHtmlEscaped());
                ++defCount;
            }
            ++meaningCount;
        }

        if(meaningCount==0) html += "<p>No online definition found.</p>";
        m_dictionaryResult->setHtml(html);
        statusBar()->showMessage("Online dictionary updated",1800);
    });
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
    if(m_document->status()==QPdfDocument::Status::Ready) finishOpenPdf();
}

void MainWindow::finishOpenPdf()
{
    if(m_currentFile.isEmpty() || m_document->status()!=QPdfDocument::Status::Ready) return;
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
    if(m_currentFile.isEmpty()){ statusBar()->showMessage("Open a PDF first.",2500); return; }
    CosSummary s=CosInspector::inspect(m_currentFile);
    auto *viewer=new QTextEdit; viewer->setReadOnly(true); viewer->setPlainText(s.humanReadable);
    auto *dialog=new QDialog(this); dialog->setAttribute(Qt::WA_DeleteOnClose); dialog->setWindowTitle("PDF COS / Physical Structure");
    auto *layout=new QVBoxLayout(dialog); layout->addWidget(viewer); dialog->resize(800,600); dialog->show();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if(m_scrollArea&&m_canvas) m_canvas->setViewportWidth(m_scrollArea->viewport()->width());
}
