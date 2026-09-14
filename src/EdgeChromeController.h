#pragma once

#include <QObject>

class MainWindow;
class QEvent;
class QToolBar;

class EdgeChromeController final : public QObject {
    Q_OBJECT
public:
    explicit EdgeChromeController(MainWindow *window, QObject *parent=nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void syncChrome();

    MainWindow *m_window=nullptr;
    QToolBar *m_toolbar=nullptr;
};
