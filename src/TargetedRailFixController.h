#pragma once
#include <QObject>

class MainWindow;
class QToolBar;
class QEvent;

class TargetedRailFixController final : public QObject {
    Q_OBJECT
public:
    explicit TargetedRailFixController(MainWindow *window, QObject *parent=nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void applyFixes();

    MainWindow *m_window=nullptr;
    QToolBar *m_toolbar=nullptr;
    bool m_applying=false;
};
