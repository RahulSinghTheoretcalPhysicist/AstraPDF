#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QKeySequence>
#include <QLabel>
#include <QSignalBlocker>

class QAction;
class QDockWidget;
class QEvent;
class QLineEdit;
class QListWidget;
class MainWindow;
class PdfCanvas;
class QPdfDocument;
class QTimer;
class QToolBar;

class ReaderSessionController final : public QObject {
    Q_OBJECT
public:
    explicit ReaderSessionController(MainWindow *window, QObject *parent=nullptr);
    void openTracked(const QString& filePath);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupOpenActionTracking();
    void buildHistoryDock();
    void refreshHistory();
    void recordOpened(const QString& filePath);
    void recordPage(int page);
    int savedPage(const QString& filePath) const;
    QString historyKey(const QString& filePath) const;
    QString normalizedPath(const QString& filePath) const;

    void buildLibraryDock();
    void addLibraryFolder();
    void removeSelectedLibraryFolder();
    void rebuildLibraryIndex();
    void refreshLibraryUi();

    void setReaderFullScreen(bool enabled);
    void showReaderChrome();
    void hideReaderChrome();
    void restartChromeTimer();

    MainWindow *m_window=nullptr;
    PdfCanvas *m_canvas=nullptr;
    QPdfDocument *m_document=nullptr;
    QToolBar *m_readerToolbar=nullptr;

    QDockWidget *m_historyDock=nullptr;
    QListWidget *m_historyList=nullptr;
    QAction *m_historyAction=nullptr;

    QDockWidget *m_libraryDock=nullptr;
    QListWidget *m_libraryFoldersList=nullptr;
    QListWidget *m_libraryFilesList=nullptr;
    QLineEdit *m_libraryFilter=nullptr;
    QAction *m_libraryAction=nullptr;

    QAction *m_fullScreenAction=nullptr;
    QTimer *m_chromeTimer=nullptr;

    QString m_currentPath;
    int m_pendingRestorePage=-1;
    bool m_readerFullScreen=false;
    bool m_restoringPage=false;
    QStringList m_visibleDockNames;
};
