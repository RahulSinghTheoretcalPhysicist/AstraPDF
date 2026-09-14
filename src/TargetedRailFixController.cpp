#include "TargetedRailFixController.h"
#include "MainWindow.h"

#include <QLabel>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

TargetedRailFixController::TargetedRailFixController(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    if(!m_window) return;
    m_toolbar=m_window->findChild<QToolBar*>("readerToolbar");
    if(!m_toolbar) return;

    applyFixes();
    QTimer::singleShot(0,this,&TargetedRailFixController::applyFixes);
}

void TargetedRailFixController::applyFixes()
{
    if(!m_toolbar) return;

    // Remove ONLY the Page block shown in the screenshot:
    // "Page" label + page-number spin box + "/ total" label.
    for(QSpinBox *spin:m_toolbar->findChildren<QSpinBox*>())
        spin->hide();

    for(QLabel *label:m_toolbar->findChildren<QLabel*>()){
        const QString text=label->text().trimmed();
        if(text.compare("Page",Qt::CaseInsensitive)==0 || text.startsWith('/'))
            label->hide();
    }

    if(QToolButton *pageChip=m_toolbar->findChild<QToolButton*>("pageNumberChip"))
        pageChip->hide();

    // Nothing else is modified here.
    m_toolbar->updateGeometry();
    m_toolbar->update();
}
