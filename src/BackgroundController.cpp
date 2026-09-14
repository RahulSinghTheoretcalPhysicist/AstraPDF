#include "BackgroundController.h"
#include "MainWindow.h"
#include "PdfCanvas.h"

#include <QAction>
#include <QColor>
#include <QColorDialog>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QStatusBar>

BackgroundController::BackgroundController(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    if(!m_window) return;
    m_canvas=m_window->findChild<PdfCanvas*>();
    if(!m_canvas) return;

    const QString saved=QSettings().value("appearance/readerBackground",QStringLiteral("#2e3033")).toString();
    applyColor(saved,false);
    setupMenu();
}

void BackgroundController::applyColor(const QString& colorName, bool persist)
{
    if(!m_canvas) return;
    const QColor color(colorName);
    if(!color.isValid()) return;

    m_canvas->setProperty("readerBackgroundColor",color);
    m_canvas->update();

    if(persist){
        QSettings s;
        s.setValue("appearance/readerBackground",color.name(QColor::HexRgb));
        s.sync();
    }

    if(m_window && m_window->statusBar())
        m_window->statusBar()->showMessage(QStringLiteral("Reader background: %1").arg(color.name(QColor::HexRgb)),1800);
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

    QMenu *backgroundMenu=viewMenu->addMenu("Reader Background");

    const struct Preset { const char *name; const char *color; } presets[] = {
        {"Dark",  "#2e3033"},
        {"Black", "#000000"},
        {"Gray",  "#5a5d61"},
        {"Sepia", "#6b5d49"},
        {"Navy",  "#18263a"},
        {"White", "#f2f2f2"}
    };

    for(const auto& preset:presets){
        QAction *action=backgroundMenu->addAction(QString::fromLatin1(preset.name));
        const QString color=QString::fromLatin1(preset.color);
        connect(action,&QAction::triggered,this,[this,color]{ applyColor(color,true); });
    }

    backgroundMenu->addSeparator();
    QAction *custom=backgroundMenu->addAction("Custom Color...");
    connect(custom,&QAction::triggered,this,[this]{
        QColor initial=m_canvas->property("readerBackgroundColor").value<QColor>();
        if(!initial.isValid()) initial=QColor("#2e3033");
        const QColor chosen=QColorDialog::getColor(initial,m_window,"Choose Reader Background Color");
        if(chosen.isValid()) applyColor(chosen.name(QColor::HexRgb),true);
    });

    QAction *reset=backgroundMenu->addAction("Reset Default");
    connect(reset,&QAction::triggered,this,[this]{ applyColor("#2e3033",true); });
}
