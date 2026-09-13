#pragma once
#include <QMainWindow>
class QAction; class QComboBox; class QDockWidget; class QLabel; class QLineEdit; class QNetworkAccessManager; class QPdfDocument;
class QResizeEvent; class QScrollArea; class QSpinBox; class QTextBrowser; class QTimer; class PdfCanvas;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent=nullptr);
    void openPdf(const QString& filePath);
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    void buildUi(); void buildActions(); void buildToolbar();
    void buildDictionaryDock();
    void applyModeFromCombo(); void updatePageUi(int page); void updateZoomUi(); void inspectCos();
    void finishOpenPdf();
    void applyTheme();
    void setNeonEnabled(bool enabled);
    void advanceNeon();
    void lookupDictionaryWord(const QString& rawWord);
    void lookupOnlineDictionaryWord(const QString& word, const QString& offlineDefinition);
    void showOfflineDictionaryWord(const QString& word, const QString& definition, bool waitingForOnline=false);

    QPdfDocument *m_document=nullptr;
    QScrollArea *m_scrollArea=nullptr;
    PdfCanvas *m_canvas=nullptr;
    QAction *m_open=nullptr,*m_copy=nullptr,*m_highlight=nullptr,*m_cosInspect=nullptr;
    QAction *m_neonAction=nullptr,*m_dictionaryAction=nullptr;
    QSpinBox *m_pageSpin=nullptr;
    QLabel *m_pageTotal=nullptr,*m_zoomLabel=nullptr;
    QComboBox *m_modeCombo=nullptr;
    QLineEdit *m_search=nullptr;
    QDockWidget *m_dictionaryDock=nullptr;
    QLineEdit *m_dictionaryInput=nullptr;
    QComboBox *m_dictionaryMode=nullptr;
    QTextBrowser *m_dictionaryResult=nullptr;
    QNetworkAccessManager *m_network=nullptr;
    QTimer *m_neonTimer=nullptr;
    bool m_neonEnabled=true;
    int m_neonHue=190;
    QString m_currentFile;
};
