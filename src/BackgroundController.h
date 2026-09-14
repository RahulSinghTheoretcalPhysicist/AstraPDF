#pragma once

#include <QObject>

class MainWindow;
class PdfCanvas;

class BackgroundController final : public QObject {
    Q_OBJECT
public:
    explicit BackgroundController(MainWindow *window, QObject *parent=nullptr);

private:
    void applyColor(const QString& colorName, bool persist=true);
    void setupMenu();

    MainWindow *m_window=nullptr;
    PdfCanvas *m_canvas=nullptr;
};
