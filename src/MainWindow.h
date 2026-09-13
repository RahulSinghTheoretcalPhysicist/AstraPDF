#pragma once
#include <QMainWindow>
class QAction; class QComboBox; class QLabel; class QLineEdit; class QPdfDocument;
class QResizeEvent; class QScrollArea; class QSpinBox; class PdfCanvas;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent=nullptr);
    void openPdf(const QString& filePath);
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    void buildUi(); void buildActions(); void buildToolbar();
    void applyModeFromCombo(); void updatePageUi(int page); void updateZoomUi(); void inspectCos();
    void finishOpenPdf();

    QPdfDocument *m_document=nullptr;
    QScrollArea *m_scrollArea=nullptr;
    PdfCanvas *m_canvas=nullptr;
    QAction *m_open=nullptr,*m_copy=nullptr,*m_highlight=nullptr,*m_cosInspect=nullptr;
    QSpinBox *m_pageSpin=nullptr;
    QLabel *m_pageTotal=nullptr,*m_zoomLabel=nullptr;
    QComboBox *m_modeCombo=nullptr;
    QLineEdit *m_search=nullptr;
    QString m_currentFile;
};
