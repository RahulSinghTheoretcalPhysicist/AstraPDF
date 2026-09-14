#include "TargetedRailFixController.h"
#include "MainWindow.h"

#include <QAction>
#include <QLabel>
#include <QLineEdit>
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

    // Exact screenshot cleanup only:
    // 1) remove Page label + page spinbox + /total
    // 2) remove Continue/page-mode rectangle
    // 3) remove Search rectangles
    // Keep zoom +/-, the visible zoom percentage label, and all other rail items.

    for(QSpinBox *spin:m_toolbar->findChildren<QSpinBox*>()) spin->hide();

    for(QLabel *label:m_toolbar->findChildren<QLabel*>()){
        const QString text=label->text().trimmed();
        if(text.compare("Page",Qt::CaseInsensitive)==0 || text.startsWith('/')){
            label->hide();
            continue;
        }

        if(text.contains('%')){
            label->setAlignment(Qt::AlignCenter);
            label->setFixedSize(34,24);
            label->show();
        }
    }

    if(QToolButton *pageChip=m_toolbar->findChild<QToolButton*>("pageNumberChip"))
        pageChip->hide();

    if(QToolButton *mode=m_toolbar->findChild<QToolButton*>("pageModeRailButton"))
        mode->hide();

    for(QLineEdit *edit:m_toolbar->findChildren<QLineEdit*>()){
        if(edit->placeholderText().contains("Search in PDF",Qt::CaseInsensitive))
            edit->hide();
    }

    for(QAction *action:m_toolbar->actions()){
        if(!action) continue;
        const QString text=action->text().trimmed();

        // The rail already has a dedicated QLabel showing the zoom percentage.
        // Keep the old QAction hidden so it cannot create a second blank/rectangular 100% button.
        if(text=="100%"){
            action->setVisible(false);
            continue;
        }

        if(text.compare("PDF Search",Qt::CaseInsensitive)==0 ||
           text.compare("Find Next",Qt::CaseInsensitive)==0 ||
           text.compare("Find",Qt::CaseInsensitive)==0 ||
           text.compare("Search",Qt::CaseInsensitive)==0 ||
           text.compare("Internet Search",Qt::CaseInsensitive)==0){
            action->setVisible(false);
        }
    }

    m_toolbar->updateGeometry();
    m_toolbar->update();
}
