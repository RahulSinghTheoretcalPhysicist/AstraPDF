#include "RailPolishController.h"
#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QCursor>
#include <QEvent>
#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

namespace {
QPixmap canvas()
{
    QPixmap pix(48,48);
    pix.fill(Qt::transparent);
    return pix;
}

QIcon cleanIcon(const QString& key)
{
    QPixmap pix=canvas();
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    const QColor cyan("#84f5ff");
    const QColor pink("#ff65d8");
    const QColor yellow("#ffd96a");
    const QColor soft("#dffcff");
    QPen pen(cyan,2.2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    if(key=="open" || key=="library" || key=="file"){
        QPainterPath path;
        path.moveTo(8,17); path.lineTo(20,17); path.lineTo(24,21); path.lineTo(40,21);
        path.lineTo(37,35); path.lineTo(10,35); path.closeSubpath();
        p.setBrush(QColor(22,92,118));
        p.drawPath(path);
        p.setBrush(QColor(34,146,181));
        p.drawRoundedRect(QRectF(10,12,15,8),2,2);
    }else if(key=="previous" || key=="next"){
        const bool right=key=="next";
        const qreal a=right?11:37, b=right?37:11;
        p.drawLine(QPointF(a,24),QPointF(b,24));
        p.drawLine(QPointF(b,24),QPointF(right?29:19,16));
        p.drawLine(QPointF(b,24),QPointF(right?29:19,32));
    }else if(key=="zoomin" || key=="zoomout" || key=="pdfsearch" || key=="websearch"){
        p.drawEllipse(QRectF(9,8,22,22));
        p.drawLine(QPointF(29,29),QPointF(39,39));
        if(key=="zoomin" || key=="zoomout"){
            p.setPen(QPen(soft,2.0,Qt::SolidLine,Qt::RoundCap));
            p.drawLine(QPointF(15,19),QPointF(25,19));
            if(key=="zoomin") p.drawLine(QPointF(20,14),QPointF(20,24));
        }else if(key=="websearch"){
            p.setPen(QPen(pink,1.35,Qt::SolidLine,Qt::RoundCap));
            p.drawEllipse(QRectF(13,12,14,14));
            p.drawLine(QPointF(20,12),QPointF(20,26));
            p.drawLine(QPointF(13,19),QPointF(27,19));
        }
    }else if(key=="history"){
        p.setPen(QPen(cyan,1.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        p.setBrush(QColor(25,79,107));
        QPainterPath left;
        left.moveTo(8,13); left.quadTo(16,10,22,14); left.lineTo(22,35); left.quadTo(16,30,8,33); left.closeSubpath();
        p.drawPath(left);
        p.setBrush(QColor(91,43,92));
        QPainterPath right;
        right.moveTo(40,13); right.quadTo(32,10,26,14); right.lineTo(26,35); right.quadTo(32,30,40,33); right.closeSubpath();
        p.drawPath(right);
        p.setPen(QPen(yellow,2.4,Qt::SolidLine,Qt::RoundCap));
        p.drawLine(QPointF(29,34),QPointF(39,24));
    }else if(key=="dictionary"){
        p.setPen(QPen(cyan,1.9,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        p.setBrush(QColor(20,67,88));
        p.drawRoundedRect(QRectF(10,10,28,29),3,3);
        p.drawLine(QPointF(24,11),QPointF(24,38));
        p.setPen(QPen(yellow,1.7));
        p.drawLine(QPointF(15,18),QPointF(21,18));
        p.drawLine(QPointF(27,18),QPointF(33,18));
    }else if(key=="continue"){
        QFont f(QStringLiteral("Segoe UI"));
        f.setPointSize(18);
        f.setBold(true);
        p.setFont(f);
        p.setPen(cyan);
        p.drawText(pix.rect(),Qt::AlignCenter,QStringLiteral("C"));
    }else if(key=="view"){
        p.setPen(QPen(cyan,2.0,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        p.setBrush(QColor(72,46,96));
        QPolygonF body;
        body << QPointF(9,19) << QPointF(31,12) << QPointF(35,22) << QPointF(13,29);
        p.drawPolygon(body);
        p.setBrush(pink);
        p.drawEllipse(QRectF(31,11,8,13));
        p.setPen(QPen(yellow,1.9,Qt::SolidLine,Qt::RoundCap));
        p.drawLine(QPointF(21,27),QPointF(16,39));
        p.drawLine(QPointF(21,27),QPointF(29,39));
    }else if(key=="copy"){
        p.drawRoundedRect(QRectF(13,11,22,25),3,3);
        p.drawRoundedRect(QRectF(9,15,22,25),3,3);
    }else if(key=="highlight"){
        p.setPen(QPen(yellow,2.5,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        p.drawLine(QPointF(13,34),QPointF(33,14));
        p.drawLine(QPointF(30,13),QPointF(36,19));
        p.drawLine(QPointF(11,37),QPointF(18,36));
    }else if(key=="inspect"){
        p.drawEllipse(QRectF(11,11,26,26));
        p.setPen(QPen(soft,2.0,Qt::SolidLine,Qt::RoundCap));
        p.drawLine(QPointF(24,21),QPointF(24,31));
        p.drawPoint(QPointF(24,16));
    }else{
        QFont f(QStringLiteral("Segoe UI Symbol"));
        f.setPointSize(15);
        f.setBold(false);
        p.setFont(f);
        p.setPen(soft);
        p.drawText(pix.rect(),Qt::AlignCenter,QStringLiteral("•"));
    }
    return QIcon(pix);
}

QString keyForButton(QToolButton *button)
{
    if(!button) return {};
    QString t=button->text().trimmed();
    if(t.isEmpty() && button->defaultAction()) t=button->defaultAction()->text().trimmed();
    const QString s=t.toLower();
    if(button->objectName()=="fileRailButton") return "file";
    if(button->objectName()=="viewRailButton") return "view";
    if(button->objectName()=="pageModeRailButton") return "continue";
    if(s.contains("open")) return "open";
    if(s.contains("previous")) return "previous";
    if(s=="next" || s.contains("next page")) return "next";
    if(s.contains("zoom in")) return "zoomin";
    if(s.contains("zoom out")) return "zoomout";
    if(s.contains("pdf search") || s=="find") return "pdfsearch";
    if(s.contains("internet search") || s=="search") return "websearch";
    if(s.contains("history")) return "history";
    if(s.contains("library")) return "library";
    if(s.contains("dictionary")) return "dictionary";
    if(s.contains("copy")) return "copy";
    if(s.contains("highlight")) return "highlight";
    if(s.contains("inspect")) return "inspect";
    return {};
}
}

RailPolishController::RailPolishController(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    if(!m_window) return;
    m_toolbar=m_window->findChild<QToolBar*>("readerToolbar");
    if(!m_toolbar) return;

    m_hoverLabel=new QLabel(m_window);
    m_hoverLabel->setObjectName("railHoverLabel");
    m_hoverLabel->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    m_hoverLabel->setStyleSheet(QStringLiteral(
        "QLabel#railHoverLabel{background:rgba(10,18,25,235);color:#f4fdff;"
        "border:1px solid rgba(120,235,245,150);border-radius:7px;"
        "padding:5px 9px;font-size:12px;font-weight:600;}"));
    m_hoverLabel->hide();

    polishRail();
    qApp->installEventFilter(this);
    QTimer::singleShot(0,this,&RailPolishController::syncRailVisibility);
}

void RailPolishController::polishRail()
{
    if(!m_toolbar) return;

    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolbar->setIconSize(QSize(22,22));
    m_toolbar->setFixedWidth(40);
    m_toolbar->setStyleSheet(QStringLiteral(
        "QToolBar#readerToolbar{background:rgba(7,13,19,224);border:none;"
        "border-right:1px solid rgba(105,235,245,90);padding:5px 3px;spacing:1px;}"
        "QToolBar#readerToolbar QToolButton{min-width:32px;max-width:32px;"
        "min-height:32px;max-height:32px;margin:0;padding:0;border:none;"
        "border-radius:8px;background:transparent;}"
        "QToolBar#readerToolbar QToolButton:hover{background:rgba(31,70,83,175);}"
        "QToolBar#readerToolbar QToolButton:pressed{background:rgba(19,48,58,220);}"
        "QToolBar#readerToolbar::separator{height:1px;background:transparent;margin:0;}"));

    QSpinBox *pageSpin=nullptr;
    QLabel *pageTotal=nullptr;
    QLabel *zoomLabel=nullptr;

    for(QComboBox *w:m_toolbar->findChildren<QComboBox*>()) w->hide();
    for(QLineEdit *w:m_toolbar->findChildren<QLineEdit*>()) w->hide();
    for(QSpinBox *w:m_toolbar->findChildren<QSpinBox*>()){
        pageSpin=w;
        w->hide();
    }
    for(QLabel *w:m_toolbar->findChildren<QLabel*>()){
        const QString text=w->text().trimmed();
        if(text.startsWith('/')) pageTotal=w;
        if(text.contains('%')) zoomLabel=w;
        w->hide();
    }

    QToolButton *pageModeButton=m_toolbar->findChild<QToolButton*>("pageModeRailButton");
    if(pageModeButton){
        pageModeButton->setText("Continue");
        pageModeButton->setToolTip("Continue");
        pageModeButton->setIcon(cleanIcon("continue"));
    }

    QToolButton *pageChip=m_toolbar->findChild<QToolButton*>("pageNumberChip");
    if(!pageChip && pageSpin){
        pageChip=new QToolButton(m_toolbar);
        pageChip->setObjectName("pageNumberChip");
        pageChip->setProperty("noExpand",true);
        pageChip->setToolButtonStyle(Qt::ToolButtonTextOnly);
        pageChip->setToolTip("Page number");
        pageChip->setFixedSize(34,26);
        pageChip->setStyleSheet("QToolButton{min-width:34px;max-width:34px;min-height:26px;max-height:26px;padding:0;border:1px solid rgba(120,235,245,90);border-radius:6px;background:rgba(8,18,25,165);color:#eaffff;font-size:10px;font-weight:600;}QToolButton:hover{border-color:#84f5ff;background:rgba(20,50,62,220);}");

        auto updatePageChip=[pageChip,pageSpin,pageTotal]{
            QString total=pageTotal?pageTotal->text().trimmed():QString();
            if(total.startsWith('/')) total=total.mid(1).trimmed();
            if(total.isEmpty()) total="0";
            pageChip->setText(QStringLiteral("%1/%2").arg(pageSpin->value()).arg(total));
        };
        updatePageChip();
        connect(pageSpin,qOverload<int>(&QSpinBox::valueChanged),pageChip,[updatePageChip](int){ updatePageChip(); });
        if(pageTotal) connect(pageTotal,&QLabel::textChanged,pageChip,[updatePageChip](const QString&){ updatePageChip(); });

        QAction *before=nullptr;
        for(QAction *a:m_toolbar->actions()){
            if(a && a->text().compare("Zoom Out",Qt::CaseInsensitive)==0){ before=a; break; }
        }
        m_toolbar->insertWidget(before,pageChip);
    }

    QToolButton *zoomChip=m_toolbar->findChild<QToolButton*>("zoomPercentChip");
    if(!zoomChip && zoomLabel){
        zoomChip=new QToolButton(m_toolbar);
        zoomChip->setObjectName("zoomPercentChip");
        zoomChip->setProperty("noExpand",true);
        zoomChip->setToolButtonStyle(Qt::ToolButtonTextOnly);
        zoomChip->setToolTip("Zoom");
        zoomChip->setFixedSize(34,24);
        zoomChip->setText(zoomLabel->text().trimmed());
        zoomChip->setStyleSheet("QToolButton{min-width:34px;max-width:34px;min-height:24px;max-height:24px;padding:0;border:none;background:transparent;color:#dffcff;font-size:10px;font-weight:600;}QToolButton:hover{background:rgba(31,70,83,175);border-radius:6px;}");
        connect(zoomLabel,&QLabel::textChanged,zoomChip,[zoomChip](const QString& text){ zoomChip->setText(text.trimmed()); });

        QAction *before=nullptr;
        for(QAction *a:m_toolbar->actions()){
            if(pageModeButton && m_toolbar->widgetForAction(a)==pageModeButton){ before=a; break; }
        }
        m_toolbar->insertWidget(before,zoomChip);
    }

    for(QToolButton *button:m_toolbar->findChildren<QToolButton*>()){
        if(button->parentWidget() && button->parentWidget()->objectName()=="windowControlStrip") continue;
        if(button->objectName()=="pageNumberChip" || button->objectName()=="zoomPercentChip") continue;
        button->setProperty("noExpand",true);
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setFixedSize(32,32);
        button->setIconSize(QSize(22,22));
        const QString key=keyForButton(button);
        if(!key.isEmpty()) button->setIcon(cleanIcon(key));
        if(key=="pdfsearch"){
            button->setText("PDF Search");
            button->setToolTip("PDF Search");
        }
        button->installEventFilter(this);
    }
}

QString RailPolishController::buttonLabel(QToolButton *button) const
{
    if(!button) return {};
    QString label=button->text().trimmed();
    if(label.isEmpty() && button->defaultAction()) label=button->defaultAction()->text().trimmed();
    if(label.isEmpty()) label=button->toolTip().trimmed();
    if(label=="Find Next" || label=="Find") label="PDF Search";
    if(label=="Search") label="Internet Search";
    return label;
}

void RailPolishController::setHovered(QToolButton *button, bool hovered)
{
    if(!button || (button->parentWidget() && button->parentWidget()->objectName()=="windowControlStrip")) return;
    if(button->objectName()=="pageNumberChip" || button->objectName()=="zoomPercentChip") return;
    button->setIconSize(hovered?QSize(28,28):QSize(22,22));
}

void RailPolishController::showHoverLabel(QToolButton *button)
{
    if(!button || !m_hoverLabel || !m_window) return;
    const QString label=buttonLabel(button);
    if(label.isEmpty()) return;

    m_hoverLabel->setText(label);
    m_hoverLabel->adjustSize();
    const QPoint anchor=button->mapTo(m_window,QPoint(button->width()+7,(button->height()-m_hoverLabel->height())/2));
    m_hoverLabel->move(anchor);
    m_hoverLabel->raise();
    m_hoverLabel->show();
}

void RailPolishController::hideHoverLabel()
{
    if(m_hoverLabel) m_hoverLabel->hide();
}

void RailPolishController::syncRailVisibility()
{
    if(!m_window || !m_toolbar) return;
    if(m_window->menuBar()) m_window->menuBar()->hide();

    m_toolbar->setFixedWidth(40);
    m_toolbar->setFixedHeight(qMax(300,m_window->height()-8));
    m_toolbar->move(0,4);

    const QPoint global=QCursor::pos();
    const QPoint local=m_window->mapFromGlobal(global);
    const bool insideWindow=m_window->rect().contains(local);
    const bool edge=insideWindow && local.x()>=0 && local.x()<=5;

    const QPoint railTopLeft=m_toolbar->mapToGlobal(QPoint(0,0));
    const QRect railRect(railTopLeft,m_toolbar->size());
    const bool overRail=m_toolbar->isVisible() && railRect.adjusted(0,-2,4,2).contains(global);
    const bool popupOpen=QApplication::activePopupWidget()!=nullptr;

    if(edge || overRail || popupOpen){
        m_toolbar->show();
        m_toolbar->raise();
    }else{
        if(m_hoveredButton){
            setHovered(m_hoveredButton,false);
            m_hoveredButton=nullptr;
        }
        hideHoverLabel();
        m_toolbar->hide();
    }
}

bool RailPolishController::eventFilter(QObject *watched, QEvent *event)
{
    if(auto *button=qobject_cast<QToolButton*>(watched)){
        if(!(button->parentWidget() && button->parentWidget()->objectName()=="windowControlStrip")){
            if(event->type()==QEvent::Enter){
                if(m_hoveredButton && m_hoveredButton!=button) setHovered(m_hoveredButton,false);
                m_hoveredButton=button;
                setHovered(button,true);
                showHoverLabel(button);
            }else if(event->type()==QEvent::Leave){
                setHovered(button,false);
                if(m_hoveredButton==button) m_hoveredButton=nullptr;
                hideHoverLabel();
            }
        }
    }

    switch(event->type()){
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::Wheel:
    case QEvent::Resize:
        QTimer::singleShot(0,this,&RailPolishController::syncRailVisibility);
        break;
    default:
        break;
    }
    return QObject::eventFilter(watched,event);
}
