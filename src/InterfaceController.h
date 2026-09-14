#pragma once

#include <QObject>

class MainWindow;
class PdfCanvas;
class QDockWidget;
class QEvent;
class QLineEdit;
class QToolBar;
class QToolButton;

class InterfaceController final : public QObject {
    Q_OBJECT
public:
    explicit InterfaceController(MainWindow *window, QObject *parent=nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void configureReaderRail();
    void configurePageModes();
    void configurePanels();
    void configureSearch();
    void configureZoomEditor();
    void compactToolbarWidgets();
    void decorateToolButtons();
    void configureWindowControls();
    void applyPanelStyle(QDockWidget *dock);
    void showRail();
    void hideRail();
    void setButtonExpanded(QToolButton *button, bool expanded);

    MainWindow *m_window=nullptr;
    PdfCanvas *m_canvas=nullptr;
    QToolBar *m_toolbar=nullptr;
    QLineEdit *m_pdfSearch=nullptr;
    QLineEdit *m_zoomEditor=nullptr;
    bool m_railVisible=false;
};
