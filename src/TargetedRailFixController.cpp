#include "TargetedRailFixController.h"
#include "MainWindow.h"

#include <QAbstractSpinBox>
#include <QAction>
#include <QColor>
#include <QComboBox>
#include <QEvent>
#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

namespace {
QIcon continueIcon()
{
    QPixmap pix(40,40);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(19,58,70));
    p.drawEllipse(QRectF(5,5,30,30));
    p.setPen(QColor("#85f6ff"));
    QFont f(QStringLiteral("Segoe UI"));
    f.setPointSize(16);
    f.setBold(true);
    p.setFont(f);
    p.drawText(pix.rect(),Qt::AlignCenter,QStringLiteral("C"));
    return QIcon(pix);
}

QIcon searchIcon()
{
    QPixmap pix(40,40);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing,true);
    QPen pen(QColor("#83f5ff"),2.4,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(7,7,20,20));
    p.drawLine(QPointF(25,25),QPointF(34,34));
    return QIcon(pix);
}
}

TargetedRailFixController::TargetedRailFixController(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    if(!m_window) return;
    m_toolbar=m_window->findChild<QToolBar*>("readerToolbar");
    if(!m_toolbar) return;

    m_toolbar->installEventFilter(this);
    applyFixes();
    QTimer::singleShot(0,this,&TargetedRailFixController::applyFixes);
    QTimer::singleShot(250,this,&TargetedRailFixController::applyFixes);
}

void TargetedRailFixController::applyFixes()
{
    if(m_applying || !m_toolbar) return;
    m_applying=true;

    // 1) Page section: keep only the actual page-number control and make it clean.
    for(QLabel *label:m_toolbar->findChildren<QLabel*>()){
        const QString text=label->text().trimmed();
        if(text.compare("Page",Qt::CaseInsensitive)==0 || text.startsWith('/')){
            label->hide();
            continue;
        }
        if(text.contains('%') || text=="100" || text=="100%"){
            label->setObjectName("targetZoomPercentLabel");
            label->setAlignment(Qt::AlignCenter);
            label->setFixedSize(36,22);
            label->setStyleSheet(QStringLiteral(
                "QLabel#targetZoomPercentLabel{background:transparent;color:#eefcff;"
                "font-size:10px;font-weight:600;padding:0;margin:0;border:none;}"));
            label->show();
        }
    }

    for(QSpinBox *spin:m_toolbar->findChildren<QSpinBox*>()){
        spin->setObjectName("targetPageNumberSpin");
        spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
        spin->setAlignment(Qt::AlignCenter);
        spin->setFixedSize(34,25);
        spin->setToolTip("Page number");
        spin->setStyleSheet(QStringLiteral(
            "QSpinBox#targetPageNumberSpin{background:rgba(10,22,29,210);color:#f4fdff;"
            "border:1px solid rgba(104,233,244,125);border-radius:6px;padding:0;"
            "font-size:11px;font-weight:600;}"
            "QSpinBox#targetPageNumberSpin:focus{border-color:#79f5ff;}"));
        spin->show();
    }

    // Hide the old mode combo/search field so they cannot leak text into the narrow rail.
    for(QComboBox *combo:m_toolbar->findChildren<QComboBox*>()) combo->hide();
    for(QLineEdit *edit:m_toolbar->findChildren<QLineEdit*>()) edit->hide();

    // 2) Continue/page-mode: compact C icon only. Existing nested menu remains intact.
    if(QToolButton *mode=m_toolbar->findChild<QToolButton*>("pageModeRailButton")){
        mode->setText("Continue");
        mode->setToolTip("Continue");
        mode->setToolButtonStyle(Qt::ToolButtonIconOnly);
        mode->setIcon(continueIcon());
        mode->setIconSize(QSize(22,22));
        mode->setFixedSize(32,32);
    }

    // 3) PDF Search: icon only in the vertical rail. Clicking it opens the existing horizontal input dialog.
    for(QAction *action:m_toolbar->actions()){
        if(!action) continue;
        const QString text=action->text().trimmed();
        if(text.compare("PDF Search",Qt::CaseInsensitive)==0 ||
           text.compare("Find Next",Qt::CaseInsensitive)==0 ||
           text.compare("Find",Qt::CaseInsensitive)==0){
            action->setText("PDF Search");
            action->setIcon(searchIcon());
            action->setToolTip("Search inside PDF");
            if(auto *button=qobject_cast<QToolButton*>(m_toolbar->widgetForAction(action))){
                button->setObjectName("targetPdfSearchButton");
                button->setToolButtonStyle(Qt::ToolButtonIconOnly);
                button->setIconSize(QSize(22,22));
                button->setFixedSize(32,32);
                button->setText(QString());
                button->setToolTip("PDF Search");
            }
        }
    }

    m_toolbar->updateGeometry();
    m_toolbar->update();
    m_applying=false;
}

bool TargetedRailFixController::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==m_toolbar && (event->type()==QEvent::Show || event->type()==QEvent::LayoutRequest))
        QTimer::singleShot(0,this,&TargetedRailFixController::applyFixes);
    return QObject::eventFilter(watched,event);
}
