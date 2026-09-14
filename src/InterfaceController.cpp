#include "InterfaceController.h"
#include "MainWindow.h"
#include "PdfCanvas.h"

#include <QAction>
#include <QActionGroup>
#include <QColor>
#include <QComboBox>
#include <QDockWidget>
#include <QEvent>
#include <QFont>
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
#include <QSpinBox>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

namespace {
QPixmap iconCanvas()
{
    QPixmap pix(44,44);
    pix.fill(Qt::transparent);
    return pix;
}

QIcon magnifierIcon(const QString& mark=QString(), bool web=false)
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    QPen pen(QColor("#7ef7ff"),3.0,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(8,7,22,22));
    p.drawLine(QPointF(27,27),QPointF(37,37));
    if(web){
        QPen thin(QColor("#ff52d9"),1.5);
        p.setPen(thin);
        p.drawArc(QRectF(11,10,16,16),0,360*16);
        p.drawLine(QPointF(19,10),QPointF(19,26));
        p.drawLine(QPointF(11,18),QPointF(27,18));
    }else if(!mark.isEmpty()){
        p.setPen(QPen(QColor("#ffffff"),2.5,Qt::SolidLine,Qt::RoundCap));
        p.drawLine(QPointF(14,18),QPointF(24,18));
        if(mark=="+") p.drawLine(QPointF(19,13),QPointF(19,23));
    }
    return QIcon(pix);
}

QIcon historyIcon()
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#82f8ff"),2.0,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(QColor("#173d63"));
    QPainterPath book;
    book.moveTo(6,10); book.quadTo(16,7,21,12); book.lineTo(21,35); book.quadTo(15,29,6,32); book.closeSubpath();
    p.drawPath(book);
    QPainterPath book2;
    book2.moveTo(38,10); book2.quadTo(28,7,23,12); book2.lineTo(23,35); book2.quadTo(29,29,38,32); book2.closeSubpath();
    p.setBrush(QColor("#5a256b"));
    p.drawPath(book2);
    p.setPen(QPen(QColor("#ffd65a"),3.0,Qt::SolidLine,Qt::RoundCap));
    p.drawLine(QPointF(28,27),QPointF(38,17));
    p.setPen(QPen(QColor("#fff3b0"),1.4));
    p.drawLine(QPointF(37,17),QPointF(39,15));
    return QIcon(pix);
}

QIcon libraryIcon()
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#8af5ff"),2));
    p.setBrush(QColor("#15527a"));
    p.drawRoundedRect(QRectF(5,13,34,23),4,4);
    p.setBrush(QColor("#1f86aa"));
    p.drawRoundedRect(QRectF(8,9,14,8),3,3);
    p.setPen(QPen(QColor("#c8fbff"),1.6));
    p.drawLine(QPointF(12,22),QPointF(32,22));
    p.drawLine(QPointF(12,27),QPointF(29,27));
    return QIcon(pix);
}

QIcon pageModeIcon()
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#7ef7ff"),2.0,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(QColor("#112a35"));
    p.drawRoundedRect(QRectF(7,7,13,28),2,2);
    p.drawRoundedRect(QRectF(23,7,13,28),2,2);
    p.setPen(QPen(QColor("#ff57d7"),3.0,Qt::SolidLine,Qt::RoundCap));
    p.drawLine(QPointF(19,31),QPointF(34,16));
    p.setPen(QPen(QColor("#ffe379"),1.8,Qt::SolidLine,Qt::RoundCap));
    p.drawLine(QPointF(33,16),QPointF(37,12));
    return QIcon(pix);
}

QIcon folderIcon()
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#89f7ff"),2.0));
    p.setBrush(QColor("#1b668a"));
    QPainterPath path;
    path.moveTo(5,14); path.lineTo(17,14); path.lineTo(21,18); path.lineTo(39,18); path.lineTo(36,35); path.lineTo(7,35); path.closeSubpath();
    p.drawPath(path);
    p.setBrush(QColor("#2ea7cf"));
    p.drawRoundedRect(QRectF(7,10,15,8),3,3);
    return QIcon(pix);
}

QIcon telescopeIcon()
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#8cf8ff"),2.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    p.setBrush(QColor("#51316f"));
    QPolygonF body;
    body << QPointF(8,17) << QPointF(30,10) << QPointF(34,20) << QPointF(12,27);
    p.drawPolygon(body);
    p.setBrush(QColor("#ff59d6"));
    p.drawEllipse(QRectF(29,9,8,13));
    p.setPen(QPen(QColor("#ffd866"),2.2,Qt::SolidLine,Qt::RoundCap));
    p.drawLine(QPointF(20,24),QPointF(16,37));
    p.drawLine(QPointF(20,24),QPointF(27,37));
    return QIcon(pix);
}

QIcon arrowIcon(bool right)
{
    QPixmap pix=iconCanvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QPen(QColor("#82f7ff"),3.0,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
    const qreal x1=right?10:34, x2=right?33:11;
    p.drawLine(QPointF(x1,22),QPointF(x2,22));
    p.drawLine(QPointF(x2,22),QPointF(right?25:19,14));
    p.drawLine(QPointF(x2,22),QPointF(right?25:19,30));
    return QIcon(pix);
}

QIcon simpleGlyph(const QString& glyph, const QColor& color=QColor("#e9fbff"), int pointSize=17)
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
    decorateToolButtons();
}

void InterfaceController::configureReaderRail()
{
    if(!m_toolbar) return;

    m_window->addToolBar(Qt::LeftToolBarArea,m_toolbar);
    m_toolbar->setOrientation(Qt::Vertical);
    m_toolbar->setMovable(false);
    m_toolbar->setFloatable(false);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolbar->setIconSize(QSize(28,28));
    m_toolbar->setMinimumWidth(52);
    m_toolbar->setMaximumWidth(148);
    m_toolbar->setStyleSheet(QStringLiteral(
        "QToolBar#readerToolbar{background:transparent;border:none;padding:5px 3px;spacing:2px;}"
        "QToolBar#readerToolbar QToolButton{min-height:42px;max-height:42px;min-width:42px;padding:0 8px;margin:1px;border:none;border-radius:12px;background:transparent;color:#eefcff;font-size:13px;font-weight:600;text-align:left;}"
        "QToolBar#readerToolbar QToolButton:hover{background:rgba(18,45,58,210);color:white;}"
        "QToolBar#readerToolbar QToolButton:pressed{background:rgba(6,25,34,235);}"
        "QToolBar#readerToolbar::separator{height:4px;background:transparent;margin:1px;}"));

    if(m_window->menuBar()){
        QMenu *fileMenu=nullptr;
        QMenu *viewMenu=nullptr;
        for(QAction *top:m_window->menuBar()->actions()){
            if(!top || !top->menu()) continue;
            if(top->text().contains("File",Qt::CaseInsensitive)) fileMenu=top->menu();
            else if(top->text().contains("View",Qt::CaseInsensitive)) viewMenu=top->menu();
        }
        if(fileMenu){
            auto *button=new QToolButton(m_toolbar);
            button->setObjectName("fileRailButton");
            button->setText("File");
            button->setIcon(folderIcon());
            button->setToolButtonStyle(Qt::ToolButtonIconOnly);
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
            button->setToolButtonStyle(Qt::ToolButtonIconOnly);
            button->setPopupMode(QToolButton::InstantPopup);
            button->setMenu(viewMenu);
            button->setToolTip("View");
            QAction *before=m_toolbar->actions().size()>1?m_toolbar->actions().at(1):nullptr;
            m_toolbar->insertWidget(before,button);
        }
        m_window->menuBar()->hide();
    }

    for(QAction *a:m_toolbar->actions()){
        if(!a) continue;
        const QString t=a->text();
        if(t.contains("Open PDF",Qt::CaseInsensitive)){
            a->setText("Open"); a->setIcon(folderIcon()); a->setToolTip("Open PDF");
        }else if(t.compare("Previous",Qt::CaseInsensitive)==0){
            a->setText("Previous"); a->setIcon(arrowIcon(false)); a->setToolTip("Previous page");
        }else if(t.compare("Next",Qt::CaseInsensitive)==0){
            a->setText("Next"); a->setIcon(arrowIcon(true)); a->setToolTip("Next page");
        }else if(t==QString::fromUtf8("−")){
            a->setText("Zoom Out"); a->setIcon(magnifierIcon("-")); a->setToolTip("Zoom Out");
        }else if(t=="+"){
            a->setText("Zoom In"); a->setIcon(magnifierIcon("+")); a->setToolTip("Zoom In");
        }else if(t.compare("Dictionary",Qt::CaseInsensitive)==0){
            a->setText("Dictionary"); a->setIcon(simpleGlyph("Aa",QColor("#ffd95f"),12)); a->setToolTip("Dictionary");
        }else if(t.compare("Search",Qt::CaseInsensitive)==0){
            a->setText("Search"); a->setIcon(magnifierIcon(QString(),true)); a->setToolTip("Internet Search");
        }else if(t.compare("History",Qt::CaseInsensitive)==0){
            a->setText("History"); a->setIcon(historyIcon()); a->setToolTip("History");
        }else if(t.contains("Library",Qt::CaseInsensitive)){
            a->setText("Library"); a->setIcon(libraryIcon()); a->setToolTip("PDF Library");
        }else if(t.compare("Copy",Qt::CaseInsensitive)==0){
            a->setText("Copy"); a->setIcon(simpleGlyph(QString::fromUtf8("⧉"),QColor("#a8f7ff"),15));
        }else if(t.compare("Highlight",Qt::CaseInsensitive)==0){
            a->setText("Highlight"); a->setIcon(simpleGlyph(QString::fromUtf8("✦"),QColor("#ffe56b"),16));
        }else if(t.contains("Inspect COS",Qt::CaseInsensitive)){
            a->setText("Inspect"); a->setIcon(simpleGlyph("i",QColor("#9df7ff"),17));
        }else if(t.compare("Min",Qt::CaseInsensitive)==0){
            a->setText("Minimize"); a->setIcon(simpleGlyph(QString::fromUtf8("−"),QColor("#9ff8ff"),18));
        }else if(t.compare("Window",Qt::CaseInsensitive)==0){
            a->setText("Window"); a->setIcon(simpleGlyph(QString::fromUtf8("□"),QColor("#9ff8ff"),18));
        }else if(t.compare("Close",Qt::CaseInsensitive)==0){
            a->setText("Close"); a->setIcon(simpleGlyph(QString::fromUtf8("×"),QColor("#ff6f9d"),18));
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
        if(combo->count()>=2 && combo->itemText(0).contains("Single Page") && combo->itemText(1).contains("Continuous")){
            modeCombo=combo;
            break;
        }
    }
    if(modeCombo){ modeCombo->setCurrentIndex(1); modeCombo->hide(); }
    else m_canvas->setViewMode(ViewMode::Continuous);

    auto *modeButton=new QToolButton(m_toolbar);
    modeButton->setObjectName("pageModeRailButton");
    modeButton->setText("Page Mode");
    modeButton->setIcon(pageModeIcon());
    modeButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    modeButton->setPopupMode(QToolButton::InstantPopup);
    modeButton->setToolTip("Page Mode");

    auto *modeMenu=new QMenu(modeButton);
    modeMenu->setStyleSheet("QMenu{background:#111a22;color:#f2fbff;border:1px solid #2a5966;border-radius:8px;padding:6px;}QMenu::item{padding:8px 24px 8px 12px;border-radius:6px;}QMenu::item:selected{background:#164555;}");
    auto *group=new QActionGroup(modeMenu);
    group->setExclusive(true);
    const struct ModeItem { const char *name; ViewMode mode; } modes[] = {
        {"Single Page",ViewMode::SinglePage},
        {"Single Continuous",ViewMode::Continuous},
        {"Double Pages",ViewMode::FacingPages},
        {"Double Continuous",ViewMode::ContinuousFacing}
    };
    for(const auto& item:modes){
        QAction *a=modeMenu->addAction(QString::fromLatin1(item.name));
        a->setCheckable(true);
        a->setChecked(item.mode==ViewMode::Continuous);
        group->addAction(a);
        connect(a,&QAction::triggered,this,[this,item,modeCombo]{
            if(modeCombo){
                int index=0;
                switch(item.mode){
                case ViewMode::SinglePage:index=0;break;
                case ViewMode::Continuous:index=1;break;
                case ViewMode::FacingPages:index=2;break;
                case ViewMode::ContinuousFacing:index=3;break;
                }
                modeCombo->setCurrentIndex(index);
            }else if(m_canvas) m_canvas->setViewMode(item.mode);
        });
    }
    modeButton->setMenu(modeMenu);
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
            connect(dock,&QDockWidget::visibilityChanged,this,[this,dock](bool visible){ if(visible) m_window->resizeDocks({dock},{430},Qt::Horizontal); });
        }else if(dock->objectName()=="historyDock"){
            dock->setMinimumWidth(350);
            connect(dock,&QDockWidget::visibilityChanged,this,[this,dock](bool visible){ if(visible) m_window->resizeDocks({dock},{390},Qt::Horizontal); });
        }
    }
}

void InterfaceController::configureSearch()
{
    if(!m_toolbar || !m_canvas) return;
    for(QLineEdit *edit:m_toolbar->findChildren<QLineEdit*>()){
        if(edit->placeholderText().contains("Search in PDF",Qt::CaseInsensitive)){
            m_pdfSearch=edit; edit->hide(); break;
        }
    }

    QAction *find=findAction(m_toolbar,"Find Next");
    if(!find) find=findAction(m_toolbar,"Find");
    if(!find) return;
    QObject::disconnect(find,nullptr,nullptr,nullptr);
    find->setIcon(magnifierIcon());
    find->setText("PDF Search");
    find->setToolTip("Search inside PDF");
    connect(find,&QAction::triggered,this,[this]{
        const QString previous=m_pdfSearch ? m_pdfSearch->text() : QString();
        bool ok=false;
        const QString text=QInputDialog::getText(m_window,"Find in PDF","Search text:",QLineEdit::Normal,previous,&ok);
        if(!ok || text.trimmed().isEmpty()) return;
        if(m_pdfSearch) m_pdfSearch->setText(text);
        m_canvas->findNext(text);
    });
}

void InterfaceController::configureZoomEditor()
{
    if(!m_toolbar || !m_canvas) return;
    m_zoomEditor=new QLineEdit(m_toolbar);
    m_zoomEditor->setObjectName("compactZoomEditor");
    m_zoomEditor->setText(QStringLiteral("%1%").arg(qRound(m_canvas->zoom()*100.0)));
    m_zoomEditor->setAlignment(Qt::AlignCenter);
    m_zoomEditor->setToolTip("Zoom percentage");
    m_zoomEditor->setFixedSize(46,28);
    m_zoomEditor->setStyleSheet("QLineEdit{font-size:11px;padding:2px;border-radius:7px;background:#0b1117;border:1px solid #2e4a55;color:#eaffff;}QLineEdit:focus{border-color:#00d9ff;}");
    m_toolbar->addWidget(m_zoomEditor);

    QAction *fit=m_toolbar->addAction(simpleGlyph(QString::fromUtf8("↔"),QColor("#83f8ff"),16),"Fit Width");
    fit->setToolTip("Fit Width");
    connect(fit,&QAction::triggered,this,[this]{
        m_canvas->fitToWidth();
        m_zoomEditor->setText(QStringLiteral("%1%").arg(qRound(m_canvas->zoom()*100.0)));
    });

    connect(m_zoomEditor,&QLineEdit::editingFinished,this,[this]{
        QString text=m_zoomEditor->text().trimmed(); text.remove('%');
        bool ok=false; const double percent=text.toDouble(&ok);
        if(ok && percent>=20.0 && percent<=500.0) m_canvas->setZoom(percent/100.0);
        m_zoomEditor->setText(QStringLiteral("%1%").arg(qRound(m_canvas->zoom()*100.0)));
    });

    for(QAction *a:m_toolbar->actions()){
        if(!a) continue;
        if(a->text()=="Zoom In" || a->text()=="Zoom Out"){
            connect(a,&QAction::triggered,this,[this]{
                QTimer::singleShot(0,this,[this]{ if(m_zoomEditor) m_zoomEditor->setText(QStringLiteral("%1%").arg(qRound(m_canvas->zoom()*100.0))); });
            });
        }
    }
}

void InterfaceController::compactToolbarWidgets()
{
    if(!m_toolbar) return;
    for(QLabel *label:m_toolbar->findChildren<QLabel*>()){
        const QString text=label->text().trimmed();
        if(text=="Page" || text.startsWith('/') || text.contains('%')) label->hide();
    }
    for(QSpinBox *spin:m_toolbar->findChildren<QSpinBox*>()){
        spin->setFixedWidth(46);
        spin->setToolTip("Page number");
        spin->setStyleSheet("QSpinBox{font-size:11px;padding:2px;background:#0b1117;border:1px solid #2e4a55;border-radius:7px;color:#eaffff;}QSpinBox:focus{border-color:#00d9ff;}");
    }
}

void InterfaceController::decorateToolButtons()
{
    if(!m_toolbar) return;
    for(QToolButton *button:m_toolbar->findChildren<QToolButton*>()){
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setFixedSize(46,44);
        button->setIconSize(QSize(28,28));
        button->installEventFilter(this);
    }
}

bool InterfaceController::eventFilter(QObject *watched, QEvent *event)
{
    if(auto *button=qobject_cast<QToolButton*>(watched)){
        if(event->type()==QEvent::Enter){
            button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            button->setIconSize(QSize(32,32));
            button->setMinimumWidth(46);
            button->setMaximumWidth(140);
            button->resize(132,44);
        }else if(event->type()==QEvent::Leave){
            button->setToolButtonStyle(Qt::ToolButtonIconOnly);
            button->setIconSize(QSize(28,28));
            button->setFixedSize(46,44);
        }
    }
    return QObject::eventFilter(watched,event);
}
