#include "TargetedRailFixController.h"
#include "MainWindow.h"

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimer>
#include <QToolBar>
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

    // Remove a widget from the toolbar layout only.
    // Do NOT delete it: MainWindow and InterfaceController still use the backing
    // objects for page tracking, page-mode changes and PDF searching.
    auto detachToolbarWidget=[this](QWidget *widget){
        if(!widget) return;
        const auto actions=m_toolbar->actions();
        for(QAction *action:actions){
            if(m_toolbar->widgetForAction(action)==widget){
                m_toolbar->removeAction(action);
                widget->hide();
                return;
            }
        }
        widget->hide();
    };

    // 1) Remove only the visible Page block: "Page" + number box + "/ total".
    for(QSpinBox *spin:m_toolbar->findChildren<QSpinBox*>())
        detachToolbarWidget(spin);

    for(QLabel *label:m_toolbar->findChildren<QLabel*>()){
        const QString text=label->text().trimmed();
        if(text.compare("Page",Qt::CaseInsensitive)==0 || text.startsWith('/'))
            detachToolbarWidget(label);
    }

    // 2) Remove only the original rectangular Continuous/Facing combo widget.
    // Keep pageModeRailButton and its Single/Double/Continuous/Facing menu intact.
    for(QComboBox *combo:m_toolbar->findChildren<QComboBox*>()){
        if(combo->count()>=4 &&
           combo->itemText(0).contains("Single Page",Qt::CaseInsensitive) &&
           combo->itemText(1).contains("Continuous",Qt::CaseInsensitive)){
            detachToolbarWidget(combo);
        }
    }

    // 3) Remove only the original rectangular "Search in PDF..." edit widget.
    // Keep PDF Search / Internet Search actions and their functionality intact.
    for(QLineEdit *edit:m_toolbar->findChildren<QLineEdit*>()){
        if(edit->placeholderText().contains("Search in PDF",Qt::CaseInsensitive))
            detachToolbarWidget(edit);
    }

    // Nothing else is hidden, deleted, disconnected or removed.
    m_toolbar->updateGeometry();
    m_toolbar->update();
}
