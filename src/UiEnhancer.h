#pragma once
#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class MainWindow;
class QDockWidget;
class QLineEdit;
class QComboBox;
class QTextBrowser;
class QPushButton;
class QNetworkAccessManager;

class UiEnhancer final : public QObject {
    Q_OBJECT
public:
    explicit UiEnhancer(MainWindow *window, QObject *parent=nullptr);

private:
    void decorateDock(QDockWidget *dock);
    void setupSearch();
    void runSearch();
    void runDuckDuckGo(const QString& query);
    void runWikipedia(const QString& query);

    MainWindow *m_window=nullptr;
    QNetworkAccessManager *m_network=nullptr;
    QDockWidget *m_webDock=nullptr;
    QLineEdit *m_webInput=nullptr;
    QComboBox *m_provider=nullptr;
    QTextBrowser *m_webResult=nullptr;
    QPushButton *m_webButton=nullptr;
    QString m_activeQuery;
};
