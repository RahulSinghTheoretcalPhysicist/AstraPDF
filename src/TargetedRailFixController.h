#pragma once
#include <QObject>

class MainWindow;
class QToolBar;

class TargetedRailFixController final : public QObject {
    Q_OBJECT
public:
    explicit TargetedRailFixController(MainWindow *window, QObject *parent=nullptr);

private:
    void applyFixes();

    MainWindow *m_window=nullptr;
    QToolBar *m_toolbar=nullptr;
};
