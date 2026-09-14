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
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

namespace {
QIcon glyphIcon(const QString& glyph, int pointSize=16)
{
    QPixmap pix(32,32);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(QColor("#e9fbff"));
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
    m_toolbar->setIconSize(QSize(21,21));
    m_toolbar->setMinimumWidth(48);
    m_toolbar->setMaximumWidth(54);
    m_toolbar->setStyleSheet(QStringLiteral(
        "QToolBar#readerToolbar{background:#10151c;border:none;border-right:1px solid #24414a;padding:5px 4px;spacing:3px;}"
        "QToolBar#readerToolbar QToolButton{min-width:36px;max-width:36px;min-height:36px;max-height:36px;padding:0;margin:1px;border:1px solid transparent;border-radius:9px;background:#151c24;color:#e9fbff;}"
        "QToolBar#readerToolbar QToolButton:hover{background:#1c2b34;border-color:#00d9ff;}"
        "QToolBar#readerToolbar QToolButton:pressed{background:#0a1117;border-color:#75efff;}"
        "QToolBar#readerToolbar::separator{height:1px;background:#29343f;margin:4px 5px;}"));

    for(QAction *a:m_toolbar->actions()){
        if(!a) continue;
        const QString t=a->text();
        if(t.contains("Open PDF",Qt::CaseInsensitive)){
            a->setIcon(m_window->style()->standardIcon(QStyle::SP_DirOpenIcon));
            a->setToolTip("Open PDF  Ctrl+O");
        }else if(t.compare("Previous",Qt::CaseInsensitive)==0){
            a->setIcon(m_window->style()->standardIcon(QStyle::SP_ArrowBack));
            a->setToolTip("Previous page");
        }else if(t.compare("Next",Qt::CaseInsensitive)==0){
            a->setIcon(m_window->style()->standardIcon(QStyle::SP_ArrowForward));
            a->setToolTip("Next page");
        }else if(t==QString::fromUtf8("−")){
            a->setIcon(glyphIcon(QString::fromUtf8("⊖"),15));
            a->setToolTip("Zoom out");
        }else if(t=="+"){
            a->setIcon(glyphIcon(QString::fromUtf8("⊕"),15));
            a->setToolTip("Zoom in");
        }else if(t.compare("Dictionary",Qt::CaseInsensitive)==0){
            a->setIcon(glyphIcon("Aa",11));
            a->setToolTip("Dictionary  Ctrl+D");
        }else if(t.compare("Search",Qt::CaseInsensitive)==0){
            a->setIcon(glyphIcon(QString::fromUtf8("⌕"),18));
            a->setToolTip("Web search  Ctrl+Shift+F");
        }else if(t.compare("History",Qt::CaseInsensitive)==0){
            a->setIcon(glyphIcon(QString::fromUtf8("↶"),17));
            a->setToolTip("Reading history  Ctrl+Shift+H");
        }else if(t.contains("Library",Qt::CaseInsensitive)){
            a->setIcon(glyphIcon(QString::fromUtf8("▣"),16));
            a->setToolTip("PDF library  Ctrl+Shift+L");
        }else if(t.compare("Copy",Qt::CaseInsensitive)==0){
            a->setIcon(glyphIcon(QString::fromUtf8("⧉"),15));
            a->setToolTip("Copy selection  Ctrl+C");
        }else if(t.compare("Highlight",Qt::CaseInsensitive)==0){
            a->setIcon(glyphIcon(QString::fromUtf8("✦"),15));
            a->setToolTip("Highlight selection");
        }else if(t.contains("Inspect COS",Qt::CaseInsensitive)){
            a->setIcon(glyphIcon("i",16));
            a->setToolTip("Inspect PDF structure");
        }else if(t.compare("Min",Qt::CaseInsensitive)==0){
            a->setIcon(glyphIcon(QString::fromUtf8("−"),17));
            a->setToolTip("Minimize");
        }else if(t.compare("Window",Qt::CaseInsensitive)==0){
            a->setIcon(glyphIcon(QString::fromUtf8("□"),17));
            a->setToolTip("Window mode");
        }else if(t.compare("Close",Qt::CaseInsensitive)==0){
            a->setIcon(glyphIcon(QString::fromUtf8("×"),18));
            a->setToolTip("Close AstraPDF");
        }else if(t=="100%"){
            a->setVisible(false);
        }
    }
}

void InterfaceController::configurePageModes()
{
    if(!m_window || !m_canvas) return;

    QComboBox *modeCombo=nullptr;
    for(QComboBox *combo:m_window->findChildren<QComboBox*>()){
        if(combo->count()>=2 && combo->itemText(0).contains("Single Page") && combo->itemText(1).contains("Continuous")){
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

    QMenu *viewMenu=nullptr;
    for(QAction *a:m_window->menuBar()->actions()){
        if(a->menu() && a->text().contains("View",Qt::CaseInsensitive)){ viewMenu=a->menu(); break; }
    }
    if(!viewMenu) return;

    QMenu *modeMenu=viewMenu->addMenu("Page Mode");
    auto *group=new QActionGroup(modeMenu);
    group->setExclusive(true);
    const struct ModeItem { const char *name; ViewMode mode; } modes[] = {
        {"Continuous",ViewMode::Continuous},
        {"Single Page",ViewMode::SinglePage},
        {"Facing Pages",ViewMode::FacingPages},
        {"Continuous Facing",ViewMode::ContinuousFacing}
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
            }else if(m_canvas){
                m_canvas->setViewMode(item.mode);
            }
        });
    }
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
    if(!find) return;
    QObject::disconnect(find,nullptr,nullptr,nullptr);
    find->setIcon(glyphIcon(QString::fromUtf8("⌕"),18));
    find->setText("Find");
    find->setToolTip("Find text in PDF");
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
    m_zoomEditor->setToolTip("Type zoom percentage, e.g. 125%");
    m_zoomEditor->setFixedSize(44,27);
    m_zoomEditor->setStyleSheet("QLineEdit{font-size:11px;padding:2px;border-radius:6px;background:#0b1117;border:1px solid #2e4a55;color:#eaffff;}QLineEdit:focus{border-color:#00d9ff;}");
    m_toolbar->addWidget(m_zoomEditor);

    QAction *fit=m_toolbar->addAction(glyphIcon(QString::fromUtf8("↔"),16),"Fit");
    fit->setToolTip("Fit page to available width");
    connect(fit,&QAction::triggered,this,[this]{
        m_canvas->fitToWidth();
        m_zoomEditor->setText(QStringLiteral("%1%").arg(qRound(m_canvas->zoom()*100.0)));
    });

    connect(m_zoomEditor,&QLineEdit::editingFinished,this,[this]{
        QString text=m_zoomEditor->text().trimmed();
        text.remove('%');
        bool ok=false;
        const double percent=text.toDouble(&ok);
        if(ok && percent>=20.0 && percent<=500.0) m_canvas->setZoom(percent/100.0);
        m_zoomEditor->setText(QStringLiteral("%1%").arg(qRound(m_canvas->zoom()*100.0)));
    });

    for(QAction *a:m_toolbar->actions()){
        if(!a) continue;
        if(a->text()=="+" || a->text()==QString::fromUtf8("−")){
            connect(a,&QAction::triggered,this,[this]{
                QTimer::singleShot(0,this,[this]{
                    if(m_zoomEditor) m_zoomEditor->setText(QStringLiteral("%1%").arg(qRound(m_canvas->zoom()*100.0)));
                });
            });
        }
    }
}

void InterfaceController::compactToolbarWidgets()
{
    if(!m_toolbar) return;
    for(QLabel *label:m_toolbar->findChildren<QLabel*>()){
        const QString text=label->text().trimmed();
        if(text=="Page" || text.startsWith('/')) label->hide();
        else if(text.contains('%')) label->hide();
    }
    for(QSpinBox *spin:m_toolbar->findChildren<QSpinBox*>()){
        spin->setFixedWidth(44);
        spin->setToolTip("Page number");
        spin->setStyleSheet("QSpinBox{font-size:11px;padding:2px;background:#0b1117;border:1px solid #2e4a55;border-radius:6px;color:#eaffff;}QSpinBox:focus{border-color:#00d9ff;}");
    }
}

void InterfaceController::decorateToolButtons()
{
    if(!m_toolbar) return;
    for(QToolButton *button:m_toolbar->findChildren<QToolButton*>()){
        button->setFixedSize(38,38);
        button->setIconSize(QSize(21,21));
        button->installEventFilter(this);
    }
}

bool InterfaceController::eventFilter(QObject *watched, QEvent *event)
{
    if(auto *button=qobject_cast<QToolButton*>(watched)){
        if(event->type()==QEvent::Enter) button->setIconSize(QSize(28,28));
        else if(event->type()==QEvent::Leave) button->setIconSize(QSize(21,21));
    }
    return QObject::eventFilter(watched,event);
}
