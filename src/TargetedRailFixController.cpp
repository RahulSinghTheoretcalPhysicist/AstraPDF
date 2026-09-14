#include "TargetedRailFixController.h"
#include "MainWindow.h"

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
    p.drawEllipse(QRectF(6,6,28,28));
    p.setPen(QColor("#85f6ff"));
    QFont f(QStringLiteral("Segoe UI"));
    f.setPointSize(15);
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
    p.drawEllipse(QRectF(8,8,19,19));
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

    // Touch only the four requested rail items. Keep every other control unchanged.

    // 1) Page number: remove the broken vertical Page / total labels and use the compact horizontal current/total chip.
    for(QLabel *label:m_toolbar->findChildren<QLabel*>()){
        const QString text=label->text().trimmed();
        if(text.compare("Page",Qt::CaseInsensitive)==0 || text.startsWith('/') || text.contains('%'))
            label->hide();
    }
    for(QSpinBox *spin:m_toolbar->findChildren<QSpinBox*>()) spin->hide();

    if(QToolButton *pageChip=m_toolbar->findChild<QToolButton*>("pageNumberChip")){
        pageChip->setToolButtonStyle(Qt::ToolButtonTextOnly);
        pageChip->setFixedSize(34,26);
        pageChip->setToolTip("Page number");
        pageChip->setStyleSheet(QStringLiteral(
            "QToolButton{min-width:34px;max-width:34px;min-height:26px;max-height:26px;"
            "padding:0;margin:0;border:1px solid rgba(120,235,245,90);border-radius:6px;"
            "background:rgba(8,18,25,165);color:#eaffff;font-size:10px;font-weight:600;}"
            "QToolButton:hover{border-color:#84f5ff;background:rgba(20,50,62,220);}"));
        pageChip->show();
    }

    // 2) Zoom 100%: keep it centered and compact, directly aligned with the rail axis.
    if(QToolButton *zoomChip=m_toolbar->findChild<QToolButton*>("zoomPercentChip")){
        zoomChip->setToolButtonStyle(Qt::ToolButtonTextOnly);
        zoomChip->setFixedSize(34,24);
        zoomChip->setToolTip("Zoom");
        zoomChip->setStyleSheet(QStringLiteral(
            "QToolButton{min-width:34px;max-width:34px;min-height:24px;max-height:24px;"
            "padding:0;margin:0;border:none;background:transparent;color:#dffcff;"
            "font-size:10px;font-weight:600;}"
            "QToolButton:hover{background:rgba(31,70,83,175);border-radius:6px;}"));
        zoomChip->show();
    }

    // 3) Continue: one compact C icon only. Existing page-mode menu/functionality stays untouched.
    if(QToolButton *mode=m_toolbar->findChild<QToolButton*>("pageModeRailButton")){
        mode->setText("Continue");
        mode->setToolTip("Continue");
        mode->setToolButtonStyle(Qt::ToolButtonIconOnly);
        mode->setIcon(continueIcon());
        mode->setIconSize(QSize(22,22));
        mode->setFixedSize(32,32);
    }

    // Old mode combo remains hidden so no duplicate Continue/Facing text appears in the rail.
    for(QComboBox *combo:m_toolbar->findChildren<QComboBox*>()) combo->hide();

    // 4) PDF Search: compact icon in the vertical rail; clicking it uses the existing horizontal search dialog.
    for(QLineEdit *edit:m_toolbar->findChildren<QLineEdit*>()) edit->hide();
    for(QAction *action:m_toolbar->actions()){
        if(!action) continue;
        const QString text=action->text().trimmed();
        if(text.compare("PDF Search",Qt::CaseInsensitive)==0 ||
           text.compare("Find Next",Qt::CaseInsensitive)==0 ||
           text.compare("Find",Qt::CaseInsensitive)==0){
            action->setText("PDF Search");
            action->setIcon(searchIcon());
            action->setToolTip("PDF Search");
            if(auto *button=qobject_cast<QToolButton*>(m_toolbar->widgetForAction(action))){
                button->setObjectName("targetPdfSearchButton");
                button->setToolButtonStyle(Qt::ToolButtonIconOnly);
                button->setIcon(searchIcon());
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
