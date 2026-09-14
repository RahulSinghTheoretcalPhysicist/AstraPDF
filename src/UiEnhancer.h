#pragma once
#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class MainWindow;
class PdfCanvas;
class QDockWidget;
class QLineEdit;
class QComboBox;
class QTextBrowser;
class QPushButton;
class QNetworkAccessManager;
class QWidget;
class QTimer;
class QMenu;

class UiEnhancer final : public QObject {
    Q_OBJECT
public:
    explicit UiEnhancer(MainWindow *window, QObject *parent=nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void decorateDock(QDockWidget *dock);
    void setupSearch();
    void setupNeonBackground();
    void setupEdgeRail();
    void positionEdgeRail();
    void showEdgeRail();
    void scheduleEdgeRailHide();
    void applyNeonBackground(const QString& colorName, bool persist=true);
    void runSearch();
    void runDuckDuckGo(const QString& query);
    void runWikipedia(const QString& query);

    MainWindow *m_window=nullptr;
    PdfCanvas *m_canvas=nullptr;
    QNetworkAccessManager *m_network=nullptr;
    QDockWidget *m_webDock=nullptr;
    QLineEdit *m_webInput=nullptr;
    QComboBox *m_provider=nullptr;
    QTextBrowser *m_webResult=nullptr;
    QPushButton *m_webButton=nullptr;
    QWidget *m_edgeRail=nullptr;
    QWidget *m_pdfSearchPopup=nullptr;
    QWidget *m_windowControls=nullptr;
    QTimer *m_edgeHideTimer=nullptr;
    QMenu *m_modeMenu=nullptr;
    QComboBox *m_modeCombo=nullptr;
    QString m_activeQuery;
};
