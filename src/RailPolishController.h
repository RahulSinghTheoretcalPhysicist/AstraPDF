#pragma once

#include <QObject>

class MainWindow;
class QEvent;
class QLabel;
class QToolBar;
class QToolButton;

class RailPolishController final : public QObject {
    Q_OBJECT
public:
    explicit RailPolishController(MainWindow *window, QObject *parent=nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void polishRail();
    void syncRailVisibility();
    void showHoverLabel(QToolButton *button);
    void hideHoverLabel();
    void setHovered(QToolButton *button, bool hovered);
    QString buttonLabel(QToolButton *button) const;

    MainWindow *m_window=nullptr;
    QToolBar *m_toolbar=nullptr;
    QLabel *m_hoverLabel=nullptr;
    QToolButton *m_hoveredButton=nullptr;
};
