#include "InterfaceController.h"
#include "MainWindow.h"
#include "PdfCanvas.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QCursor>
#include <QDockWidget>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

namespace {
QPixmap iconCanvas()
{
    QPixmap pix(40,40);
    pix.fill(Qt::transparent);
    return pix;
}

QIcon magnifierIcon(const QString& mark=QString(), bool web=false)
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#75f4ff"),2.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(7,6,20,20));
    p.drawLine(QPointF(25,25),QPointF(34,34));
    if(web){
        p.setPen(QPen(QColor("#ff5bd8"),1.3));
        p.drawEllipse(QRectF(10,9,14,14));
        p.drawLine(QPointF(17,9),QPointF(17,23));
        p.drawLine(QPointF(10,16),QPointF(24,16));
    }else if(!mark.isEmpty()){
        p.setPen(QPen(Qt::white,2.2,Qt::SolidLine,Qt::RoundCap));
        p.drawLine(QPointF(12,16),QPointF(22,16));
        if(mark=="+") p.drawLine(QPointF(17,11),QPointF(17,21));
    }
    return QIcon(pix);
}

QIcon historyIcon()
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#7ef6ff"),1.7,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(QColor("#185579"));
    QPainterPath left;
    left.moveTo(5,10); left.quadTo(13,7,19,11); left.lineTo(19,32); left.quadTo(13,27,5,30); left.closeSubpath();
    p.drawPath(left);
    p.setBrush(QColor("#71356f"));
    QPainterPath right;
    right.moveTo(35,10); right.quadTo(27,7,21,11); right.lineTo(21,32); right.quadTo(27,27,35,30); right.closeSubpath();
    p.drawPath(right);
    p.setPen(QPen(QColor("#ffd85a"),2.7,Qt::SolidLine,Qt::RoundCap));
    p.drawLine(QPointF(25,29),QPointF(35,19));
    return QIcon(pix);
}

QIcon pageModeIcon()
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#7df5ff"),1.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(QColor("#102a35"));
    p.drawRoundedRect(QRectF(6,7,12,26),2,2);
    p.drawRoundedRect(QRectF(21,7,12,26),2,2);
    p.setPen(QPen(QColor("#ff56d5"),2.6,Qt::SolidLine,Qt::RoundCap));
    p.drawLine(QPointF(18,29),QPointF(31,16));
    p.setPen(QPen(QColor("#ffe16b"),1.7,Qt::SolidLine,Qt::RoundCap));
    p.drawLine(QPointF(30,16),QPointF(34,12));
    return QIcon(pix);
}

QIcon folderIcon()
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#7cf4ff"),1.8));
    p.setBrush(QColor("#1b7699"));
    QPainterPath path;
    path.moveTo(5,14); path.lineTo(15,14); path.lineTo(19,18); path.lineTo(35,18); path.lineTo(33,32); path.lineTo(7,32); path.closeSubpath();
    p.drawPath(path);
    p.setBrush(QColor("#2aa8cf"));
    p.drawRoundedRect(QRectF(7,10,14,7),2,2);
    return QIcon(pix);
}

QIcon telescopeIcon()
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#80f6ff"),2.0,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(QColor("#56366f"));
    QPolygonF body;
    body << QPointF(7,16) << QPointF(27,10) << QPointF(31,19) << QPointF(11,25);
    p.drawPolygon(body);
    p.setBrush(QColor("#ff5ad7"));
    p.drawEllipse(QRectF(27,9,7,11));
    p.setPen(QPen(QColor("#ffd963"),1.9,Qt::SolidLine,Qt::RoundCap));
    p.drawLine(QPointF(18,23),QPointF(14,34));
    p.drawLine(QPointF(18,23),QPointF(25,34));
    return QIcon(pix);
}

QIcon arrowIcon(bool right)
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#7ef6ff"),2.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    const qreal x1=right?9:31, x2=right?31:9;
    p.drawLine(QPointF(x1,20),QPointF(x2,20));
    p.drawLine(QPointF(x2,20),QPointF(right?24:16,13));
    p.drawLine(QPointF(x2,20),QPointF(right?24:16,27));
    return QIcon(pix);
}

QIcon simpleGlyph(const QString& glyph, const QColor& color=QColor("#e9fbff"), int pointSize=15)
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(color);
    QFont f(QStringLiteral("Segoe UI Symbol"));
    f.setPointSize(pointSize);
    f.setBold(true);
    p.setFont(f);
    p.drawText(pix.rect(),Qt::AlignCenter,glyph);
    return QIcon(pix);
}

QAction *findAction(QToolBar *toolbar, const QString& text)
{
    if(!toolbar) return nullptr;
    for(QAction *a:toolbar->actions())
        if(a && a->text().compare(text,Qt::CaseInsensitive)==0) return a;
    return nullptr;
}
}

InterfaceController::InterfaceController(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    if(!m_window) return;
    m_canvas=m_window->findChild<PdfCanvas*>();
    m_toolbar=m_window->findChild<QToolBar*>("readerToolbar");

    configureReaderRail();
    configurePageModes();
    configurePanels();
    configureSearch();
    configureZoomEditor();
    compactToolbarWidgets();
    configureWindowControls();
    decorateToolButtons();

    qApp->installEventFilter(this);
    QTimer::singleShot(0,this,[this]{ hideRail(); });
}

void InterfaceController::configureReaderRail()
{
    if(!m_toolbar) return;

    m_window->removeToolBar(m_toolbar);
    m_toolbar->setParent(m_window);
    m_toolbar->setWindowFlags(Qt::Widget);
    m_toolbar->setOrientation(Qt::Vertical);
    m_toolbar->setMovable(false);
    m_toolbar->setFloatable(false);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolbar->setIconSize(QSize(24,24));
    m_toolbar->setMinimumWidth(48);
    m_toolbar->setMaximumWidth(142);
    m_toolbar->setStyleSheet(QStringLiteral(
        "QToolBar#readerToolbar{background:transparent;border:none;padding:4px 3px;spacing:1px;}"
        "QToolBar#readerToolbar QToolButton{min-height:36px;max-height:36px;min-width:38px;padding:0 6px;margin:0;border:none;border-radius:10px;background:rgba(8,18,25,115);color:#eefcff;font-size:12px;font-weight:600;text-align:left;}"
        "QToolBar#readerToolbar QToolButton:hover{background:rgba(18,48,60,220);color:white;}"
        "QToolBar#readerToolbar QToolButton:pressed{background:rgba(5,20,28,235);}"
        "QToolBar#readerToolbar::separator{height:2px;background:transparent;margin:0;}"));

    QMenu *fileMenu=nullptr;
    QMenu *viewMenu=nullptr;
    if(m_window->menuBar()){
        for(QAction *top:m_window->menuBar()->actions()){
            if(!top || !top->menu()) continue;
            if(top->text().contains("File",Qt::CaseInsensitive)) fileMenu=top->menu();
            else if(top->text().contains("View",Qt::CaseInsensitive)) viewMenu=top->menu();
        }
    }

    if(fileMenu){
        auto *button=new QToolButton(m_toolbar);
        button->setObjectName("fileRailButton");
        button->setText("File");
        button->setIcon(folderIcon());
        button->setPopupMode(QToolButton::InstantPopup);
        button->setMenu(fileMenu);
        button->setToolTip("File");
        m_toolbar->insertWidget(m_toolbar->actions().isEmpty()?nullptr:m_toolbar->actions().first(),button);
    }
    if(viewMenu){
        auto *button=new QToolButton(m_toolbar);
        button->setObjectName("viewRailButton");
        button->setText("View");
        button->setIcon(telescopeIcon());
        button->setPopupMode(QToolButton::InstantPopup);
        button->setMenu(viewMenu);
        button->setToolTip("View");
        QAction *before=m_toolbar->actions().size()>1?m_toolbar->actions().at(1):nullptr;
        m_toolbar->insertWidget(before,button);
    }
    if(m_window->menuBar()) m_window->menuBar()->hide();

    for(QAction *a:m_toolbar->actions()){
        if(!a) continue;
        const QString t=a->text();
        if(t.contains("Open PDF",Qt::CaseInsensitive)){
            a->setText("Open PDF"); a->setIcon(folderIcon());
        }else if(t.compare("Previous",Qt::CaseInsensitive)==0){
            a->setIcon(arrowIcon(false));
        }else if(t.compare("Next",Qt::CaseInsensitive)==0){
            a->setIcon(arrowIcon(true));
        }else if(t==QString::fromUtf8("−")){
            a->setText("Zoom Out"); a->setIcon(magnifierIcon("-"));
        }else if(t=="+"){
            a->setText("Zoom In"); a->setIcon(magnifierIcon("+"));
        }else if(t.compare("Dictionary",Qt::CaseInsensitive)==0){
            a->setIcon(simpleGlyph("Aa",QColor("#ffd95f"),11));
        }else if(t.compare("Search",Qt::CaseInsensitive)==0){
            a->setText("Internet Search"); a->setIcon(magnifierIcon(QString(),true));
        }else if(t.compare("History",Qt::CaseInsensitive)==0){
            a->setIcon(historyIcon());
        }else if(t.contains("Library",Qt::CaseInsensitive)){
            a->setText("Library"); a->setIcon(folderIcon());
        }else if(t.compare("Copy",Qt::CaseInsensitive)==0){
            a->setIcon(simpleGlyph(QString::fromUtf8("⧉"),QColor("#a8f7ff"),14));
        }else if(t.compare("Highlight",Qt::CaseInsensitive)==0){
            a->setIcon(simpleGlyph(QString::fromUtf8("✦"),QColor("#ffe56b"),15));
        }else if(t.contains("Inspect COS",Qt::CaseInsensitive)){
            a->setText("Inspect"); a->setIcon(simpleGlyph("i",QColor("#9df7ff"),16));
        }else if(t=="100%"){
            a->setVisible(false);
        }
    }
}

void InterfaceController::configurePageModes()
{
    if(!m_window || !m_canvas || !m_toolbar) return;

    QComboBox *modeCombo=nullptr;
    for(QComboBox *combo:m_window->findChildren<QComboBox*>()){
        if(combo->count()>=4 && combo->itemText(0).contains("Single Page",Qt::CaseInsensitive)){
            modeCombo=combo;
            break;
        }
    }
    if(modeCombo){
        modeCombo->setCurrentIndex(1);
        modeCombo->hide();
    }else{
        m_canvas->setViewMode(ViewMode::Continuous);
    }

    auto applyMode=[this,modeCombo](ViewMode mode){
        if(modeCombo){
            int index=1;
            switch(mode){
            case ViewMode::SinglePage:index=0;break;
            case ViewMode::Continuous:index=1;break;
            case ViewMode::FacingPages:index=2;break;
            case ViewMode::ContinuousFacing:index=3;break;
            }
            modeCombo->setCurrentIndex(index);
        }else if(m_canvas){
            m_canvas->setViewMode(mode);
        }
    };

    auto *modeButton=new QToolButton(m_toolbar);
    modeButton->setObjectName("pageModeRailButton");
    modeButton->setText("Page Mode");
    modeButton->setIcon(pageModeIcon());
    modeButton->setPopupMode(QToolButton::InstantPopup);
    modeButton->setToolTip("Page Mode");

    auto *root=new QMenu(modeButton);
    auto *continuous=root->addMenu("Continuous");
    QAction *continuousSingle=continuous->addAction("Single page");
    QAction *continuousDouble=continuous->addAction("Double page");
    connect(continuousSingle,&QAction::triggered,this,[applyMode]{ applyMode(ViewMode::Continuous); });
    connect(continuousDouble,&QAction::triggered,this,[applyMode]{ applyMode(ViewMode::ContinuousFacing); });

    auto *facing=root->addMenu("Facing");
    QAction *facingSingle=facing->addAction("Single page");
    QAction *facingDouble=facing->addAction("Double page");
    connect(facingSingle,&QAction::triggered,this,[applyMode]{ applyMode(ViewMode::SinglePage); });
    connect(facingDouble,&QAction::triggered,this,[applyMode]{ applyMode(ViewMode::FacingPages); });

    modeButton->setMenu(root);
    m_toolbar->addWidget(modeButton);
}

void InterfaceController::applyPanelStyle(QDockWidget *dock)
{
    if(!dock) return;
    dock->setStyleSheet(QStringLiteral(
        "QDockWidget{color:#eefaff;font-size:14px;font-weight:600;}"
        "QDockWidget QWidget{background:#111820;color:#eaf5f8;}"
        "QLabel{font-size:13px;color:#b9cbd2;}"
        "QListWidget{background:#0b1117;border:1px solid #28424d;border-radius:10px;font-size:14px;padding:6px;outline:0;}"
        "QListWidget::item{padding:10px 8px;margin:2px;border-radius:7px;}"
        "QListWidget::item:hover{background:#172832;}"
        "QListWidget::item:selected{background:#174353;border:1px solid #00d9ff;color:white;}"
        "QLineEdit{background:#0b1117;border:1px solid #304854;border-radius:8px;padding:7px;font-size:13px;color:white;}"
        "QPushButton{background:#18222b;border:1px solid #334b56;border-radius:7px;padding:6px 9px;font-size:12px;color:#eaf5f8;}"
        "QPushButton:hover{background:#1c3844;border-color:#00d9ff;}"));
}

void InterfaceController::configurePanels()
{
    if(!m_window) return;
    for(QDockWidget *dock:m_window->findChildren<QDockWidget*>()){
        applyPanelStyle(dock);
        if(dock->objectName()=="libraryDock"){
            dock->setMinimumWidth(380);
            connect(dock,&QDockWidget::visibilityChanged,this,[this,dock](bool visible){
                if(visible) m_window->resizeDocks({dock},{430},Qt::Horizontal);
            });
        }else if(dock->objectName()=="historyDock"){
            dock->setMinimumWidth(350);
            connect(dock,&QDockWidget::visibilityChanged,this,[this,dock](bool visible){
                if(visible) m_window->resizeDocks({dock},{390},Qt::Horizontal);
            });
        }
    }
}

void InterfaceController::configureSearch()
{
    if(!m_toolbar || !m_canvas) return;

    for(QLineEdit *edit:m_toolbar->findChildren<QLineEdit*>()){
        if(edit->placeholderText().contains("Search in PDF",Qt::CaseInsensitive)){
            m_pdfSearch=edit;
            edit->hide();
            break;
        }
    }

    QAction *find=findAction(m_toolbar,"Find Next");
    if(!find) find=findAction(m_toolbar,"Find");
    if(!find) return;

    QObject::disconnect(find,nullptr,nullptr,nullptr);
    find->setText("PDF Search");
    find->setIcon(magnifierIcon());
    find->setToolTip("Search inside PDF");
    connect(find,&QAction::triggered,this,[this]{
        const QString previous=m_pdfSearch?m_pdfSearch->text():QString();
        bool ok=false;
        const QString text=QInputDialog::getText(m_window,"Search in PDF","Search text:",QLineEdit::Normal,previous,&ok);
        if(!ok || text.trimmed().isEmpty()) return;
        if(m_pdfSearch) m_pdfSearch->setText(text);
        m_canvas->findNext(text);
    });
}

void InterfaceController::configureZoomEditor()
{
    if(!m_window || !m_canvas) return;

    if(m_zoomEditor) m_zoomEditor->hide();
    for(QLineEdit *edit:m_toolbar->findChildren<QLineEdit*>()){
        if(edit->objectName()=="compactZoomEditor") edit->hide();
    }

    QMenu *viewMenu=nullptr;
    if(m_window->menuBar()){
        for(QAction *a:m_window->menuBar()->actions()){
            if(a->menu() && a->text().contains("View",Qt::CaseInsensitive)){
                viewMenu=a->menu();
                break;
            }
        }
    }
    if(!viewMenu) return;

    QAction *setZoom=viewMenu->addAction("Set Zoom...");
    connect(setZoom,&QAction::triggered,this,[this]{
        bool ok=false;
        const int current=qRound(m_canvas->zoom()*100.0);
        const int percent=QInputDialog::getInt(m_window,"Set Zoom","Zoom percentage:",current,20,500,5,&ok);
        if(ok) m_canvas->setZoom(percent/100.0);
    });
    QAction *fitWidth=viewMenu->addAction("Fit Width");
    connect(fitWidth,&QAction::triggered,m_canvas,&PdfCanvas::fitToWidth);
}

void InterfaceController::compactToolbarWidgets()
{
    if(!m_toolbar) return;

    for(QLabel *label:m_toolbar->findChildren<QLabel*>()){
        const QString text=label->text().trimmed();
        if(text=="Page" || text.startsWith('/') || text.contains('%')) label->hide();
    }
    for(QComboBox *combo:m_toolbar->findChildren<QComboBox*>()) combo->hide();
    for(QLineEdit *edit:m_toolbar->findChildren<QLineEdit*>()) edit->hide();

    for(QSpinBox *spin:m_toolbar->findChildren<QSpinBox*>()){
        spin->setFixedSize(38,27);
        spin->setToolTip("Page number");
        spin->setStyleSheet("QSpinBox{font-size:11px;padding:1px;background:rgba(8,18,25,180);border:1px solid #2e4a55;border-radius:6px;color:#eaffff;}QSpinBox:focus{border-color:#00d9ff;}");
    }
}

void InterfaceController::configureWindowControls()
{
    if(!m_toolbar) return;

    QAction *minAction=nullptr;
    QAction *windowAction=nullptr;
    QAction *closeAction=nullptr;
    for(QAction *a:m_toolbar->actions()){
        if(!a) continue;
        const QString text=a->text();
        if(text.compare("Min",Qt::CaseInsensitive)==0 || text.compare("Minimize",Qt::CaseInsensitive)==0) minAction=a;
        else if(text.compare("Window",Qt::CaseInsensitive)==0) windowAction=a;
        else if(text.compare("Close",Qt::CaseInsensitive)==0) closeAction=a;
    }
    if(minAction) minAction->setVisible(false);
    if(windowAction) windowAction->setVisible(false);
    if(closeAction) closeAction->setVisible(false);

    auto *spacer=new QWidget(m_toolbar);
    spacer->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding);
    m_toolbar->addWidget(spacer);

    auto *strip=new QWidget(m_toolbar);
    strip->setObjectName("windowControlStrip");
    auto *layout=new QHBoxLayout(strip);
    layout->setContentsMargins(1,0,1,0);
    layout->setSpacing(1);

    auto addControl=[this,layout,strip](const QIcon& icon,const QString& tip,QAction *action){
        auto *button=new QToolButton(strip);
        button->setProperty("noExpand",true);
        button->setIcon(icon);
        button->setIconSize(QSize(12,12));
        button->setFixedSize(15,17);
        button->setToolTip(tip);
        button->setStyleSheet("QToolButton{border:none;background:transparent;padding:0;}QToolButton:hover{background:rgba(50,90,105,210);border-radius:4px;}");
        if(action) connect(button,&QToolButton::clicked,action,&QAction::trigger);
        layout->addWidget(button);
    };

    addControl(simpleGlyph(QString::fromUtf8("−"),QColor("#a6f8ff"),13),"Minimize",minAction);
    addControl(simpleGlyph(QString::fromUtf8("□"),QColor("#a6f8ff"),12),"Window",windowAction);
    addControl(simpleGlyph(QString::fromUtf8("×"),QColor("#ff718f"),13),"Close",closeAction);
    m_toolbar->addWidget(strip);
}

void InterfaceController::decorateToolButtons()
{
    if(!m_toolbar) return;
    for(QToolButton *button:m_toolbar->findChildren<QToolButton*>()){
        if(button->property("noExpand").toBool()) continue;
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setFixedSize(40,36);
        button->setIconSize(QSize(24,24));
        button->installEventFilter(this);
    }
}

void InterfaceController::showRail()
{
    if(!m_toolbar || !m_window) return;
    m_toolbar->setFixedWidth(48);
    m_toolbar->setFixedHeight(qMax(300,m_window->height()-12));
    m_toolbar->move(3,6);
    m_toolbar->show();
    m_toolbar->raise();
    m_railVisible=true;
}

void InterfaceController::hideRail()
{
    if(!m_toolbar) return;
    if(QApplication::activePopupWidget()) return;
    for(QToolButton *button:m_toolbar->findChildren<QToolButton*>())
        if(!button->property("noExpand").toBool()) setButtonExpanded(button,false);
    m_toolbar->hide();
    m_railVisible=false;
}

void InterfaceController::setButtonExpanded(QToolButton *button, bool expanded)
{
    if(!button || button->property("noExpand").toBool()) return;
    if(expanded){
        m_toolbar->setFixedWidth(140);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setFixedSize(134,36);
        button->setIconSize(QSize(30,30));
    }else{
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setFixedSize(40,36);
        button->setIconSize(QSize(24,24));
        bool anyExpanded=false;
        for(QToolButton *other:m_toolbar->findChildren<QToolButton*>()){
            if(other!=button && !other->property("noExpand").toBool() && other->toolButtonStyle()==Qt::ToolButtonTextBesideIcon){
                anyExpanded=true;
                break;
            }
        }
        if(!anyExpanded) m_toolbar->setFixedWidth(48);
    }
}

bool InterfaceController::eventFilter(QObject *watched, QEvent *event)
{
    if(auto *button=qobject_cast<QToolButton*>(watched)){
        if(!button->property("noExpand").toBool()){
            if(event->type()==QEvent::Enter){
                showRail();
                for(QToolButton *other:m_toolbar->findChildren<QToolButton*>())
                    if(other!=button && !other->property("noExpand").toBool()) setButtonExpanded(other,false);
                setButtonExpanded(button,true);
            }else if(event->type()==QEvent::Leave){
                setButtonExpanded(button,false);
            }
        }
    }

    if(event->type()==QEvent::MouseMove && m_window && m_toolbar){
        const QPoint global=QCursor::pos();
        const QPoint local=m_window->mapFromGlobal(global);
        const bool insideWindow=m_window->rect().contains(local);
        const bool atLeftEdge=insideWindow && local.x()>=0 && local.x()<=7;
        if(atLeftEdge && !m_railVisible) showRail();

        if(m_railVisible){
            const QPoint topLeft=m_toolbar->mapToGlobal(QPoint(0,0));
            const QRect railRect(topLeft,m_toolbar->size());
            if(!railRect.adjusted(-3,-3,8,3).contains(global) && !QApplication::activePopupWidget()) hideRail();
        }
    }else if(event->type()==QEvent::Resize && watched==m_window && m_railVisible){
        showRail();
    }

    return QObject::eventFilter(watched,event);
}
