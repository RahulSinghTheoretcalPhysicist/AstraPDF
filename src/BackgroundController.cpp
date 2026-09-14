#include "BackgroundController.h"
#include "MainWindow.h"
#include "PdfCanvas.h"

#include <QAction>
#include <QColor>
#include <QColorDialog>
#include <QMenu>
#include <QMenuBar>
#include <QScrollArea>
#include <QSettings>
#include <QStatusBar>

BackgroundController::BackgroundController(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    if(!m_window) return;
    m_canvas=m_window->findChild<PdfCanvas*>();
    if(!m_canvas) return;

    const QString saved=QSettings().value("appearance/readerBackground",QStringLiteral("#00e5ff")).toString();
    applyColor(saved,false);
    setupMenu();
}

void BackgroundController::applyColor(const QString& colorName, bool persist)
{
    if(!m_canvas) return;
    const QColor color(colorName);
    if(!color.isValid()) return;

    m_canvas->setNeonBackground(color);
    m_canvas->setProperty("readerBackgroundColor",color);

    if(m_window){
        if(auto *scroll=m_window->findChild<QScrollArea*>("documentArea")){
            int hue=color.hsvHue();
            if(hue<0) hue=190;
            const QColor deep=QColor::fromHsv(hue,155,22);
            scroll->setStyleSheet(QStringLiteral("QScrollArea#documentArea{background:%1;border:none;}QScrollArea#documentArea QWidget#qt_scrollarea_viewport{background:%1;}").arg(deep.name()));
        }
    }

    if(persist){
        QSettings s;
        s.setValue("appearance/readerBackground",color.name(QColor::HexRgb));
        s.sync();
    }

    if(m_window && m_window->statusBar())
        m_window->statusBar()->showMessage(QStringLiteral("Neon reader background: %1").arg(color.name(QColor::HexRgb)),1800);
}

void BackgroundController::setupMenu()
{
    if(!m_window || !m_canvas) return;

    QMenu *viewMenu=nullptr;
    for(QAction *action:m_window->menuBar()->actions()){
        if(action->menu() && action->text().contains("View",Qt::CaseInsensitive)){
            viewMenu=action->menu();
            break;
        }
    }
    if(!viewMenu) viewMenu=m_window->menuBar()->addMenu("&View");

    QMenu *backgroundMenu=viewMenu->addMenu("Neon Reader Background");

    const struct Preset { const char *name; const char *color; } presets[] = {
        {"Neon Cyan",    "#00e5ff"},
        {"Electric Blue","#2979ff"},
        {"Neon Purple",  "#b388ff"},
        {"Neon Magenta", "#ff2bd6"},
        {"Neon Green",   "#39ff88"},
        {"Amber",        "#ffb300"},
        {"Graphite",     "#5a6570"}
    };

    for(const auto& preset:presets){
        QAction *action=backgroundMenu->addAction(QString::fromLatin1(preset.name));
        const QString color=QString::fromLatin1(preset.color);
        connect(action,&QAction::triggered,this,[this,color]{ applyColor(color,true); });
    }

    backgroundMenu->addSeparator();
    QAction *custom=backgroundMenu->addAction("Custom Neon Color...");
    connect(custom,&QAction::triggered,this,[this]{
        QColor initial=m_canvas->neonBackgroundColor();
        if(!initial.isValid()) initial=QColor("#00e5ff");
        const QColor chosen=QColorDialog::getColor(initial,m_window,"Choose Neon Reader Background");
        if(chosen.isValid()) applyColor(chosen.name(QColor::HexRgb),true);
    });

    QAction *reset=backgroundMenu->addAction("Reset Neon Cyan");
    connect(reset,&QAction::triggered,this,[this]{ applyColor("#00e5ff",true); });
}
