#include "TargetedRailFixController.h"
#include "MainWindow.h"

#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

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

    auto removeToolbarWidget=[this](QWidget *widget){
        if(!widget) return;
        const auto actions=m_toolbar->actions();
        for(QAction *action:actions){
            if(m_toolbar->widgetForAction(action)==widget){
                m_toolbar->removeAction(action);
                action->deleteLater();
                break;
            }
        }
        widget->deleteLater();
    };

    // Page UI: remove the entire Page / number / total group from the rail.
    const auto spins=m_toolbar->findChildren<QSpinBox*>();
    for(QSpinBox *spin:spins)
        removeToolbarWidget(spin);

    const auto labels=m_toolbar->findChildren<QLabel*>();
    for(QLabel *label:labels){
        const QString text=label->text().trimmed();
        if(text.compare("Page",Qt::CaseInsensitive)==0 || text.startsWith('/'))
            removeToolbarWidget(label);
    }

    if(QToolButton *pageChip=m_toolbar->findChild<QToolButton*>("pageNumberChip"))
        removeToolbarWidget(pageChip);

    // Page-mode/Continue control: do not keep a rail widget for it.
    if(QToolButton *mode=m_toolbar->findChild<QToolButton*>("pageModeRailButton"))
        removeToolbarWidget(mode);

    // PDF search edit box: remove the widget from the rail.
    const auto edits=m_toolbar->findChildren<QLineEdit*>();
    for(QLineEdit *edit:edits){
        if(edit->placeholderText().contains("Search in PDF",Qt::CaseInsensitive))
            removeToolbarWidget(edit);
    }

    // Remove only search-related rail actions. Other actions remain untouched.
    const auto actions=m_toolbar->actions();
    for(QAction *action:actions){
        if(!action) continue;
        const QString text=action->text().trimmed();
        if(text.compare("PDF Search",Qt::CaseInsensitive)==0 ||
           text.compare("Find Next",Qt::CaseInsensitive)==0 ||
           text.compare("Find",Qt::CaseInsensitive)==0 ||
           text.compare("Search",Qt::CaseInsensitive)==0 ||
           text.compare("Internet Search",Qt::CaseInsensitive)==0){
            m_toolbar->removeAction(action);
        }
    }

    m_toolbar->updateGeometry();
    m_toolbar->update();
}
