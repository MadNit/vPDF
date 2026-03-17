#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QSplitter>
#include <QToolBar>
#include <QAction>
#include <QProgressBar>
#include <QTabWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <memory>
#include <QList>

#include "engine/IPdfEngine.h"
#include "ui/PageViewer.h"
#include "ui/ThumbnailPanel.h"
#include "ui/SidebarMenu.h"
#include "ui/CollapsiblePanel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    // File
    void onOpenFile();
    void onSaveEditedAs();
    void onCloseFile();

    // Operations
    void onMerge();
    void onSplit();
    void onCompress();
    void onExtractText();
    void onEncrypt();
    void onDecrypt();
    void onRotateLeft();
    void onRotateRight();
    void onExportToImages();
    void onImportFromImages();

    // Navigation
    void onPageChanged(int pageIndex);
    void onZoomIn();
    void onZoomOut();
    void onFitWidth();

    // Thumbnail panel actions
    void onThumbPagesReordered(const QList<int>& orderedPageIndices);
    void onThumbRotateLeft(int pageIndex);
    void onThumbRotateRight(int pageIndex);
    void onThumbDelete(int pageIndex);
    void onThumbCopyRequested(const QList<int>& pageIndices);
    void onThumbCutRequested(const QList<int>& pageIndices);
    void onThumbPasteRequested(int insertAfterPageIndex);

private:
    struct DocContext;

    void setupMenuBar();
    void setupToolBar();
    void setupCentralWidget();
    void setupSidebar();
    void setupStatusBar();

    void loadDocument(const QString& filePath);
    void updateWindowTitle();
    void setOperationsEnabled(bool enabled);
    void updateOperationsState();

    // Edit session model
    QString activeDocumentPath(const DocContext* ctx) const;
    bool ensureWorkingCopy(DocContext* ctx);
    bool saveEditedAs(DocContext* ctx, const QString& outPath);
    bool maybeSaveEdits(DocContext* ctx);
    void cleanupWorkingCopy(DocContext& ctx);

    bool createNextWorkingCopy(DocContext* ctx, QString& prevPath, QString& nextPath);
    bool applyEditedPath(DocContext* ctx, const QString& prevPath, const QString& nextPath, int pageToShow);
    void rotatePageAt(int pageIndex, int deltaDegrees);
    void deletePageAt(int pageIndex);
    void reorderPagesTo(const QList<int>& orderedPageIndices);
    QList<int> normalizePageIndices(const QList<int>& pageIndices, int totalPages) const;

    // Tab Management
    void onTabChanged(int index);
    void onTabCloseRequested(int index);

    struct DocContext {
        QString sourcePath;     // immutable input
        QString workingPath;    // temp editable copy
        QString exportPath;     // last exported location
        bool dirty = false;

        std::shared_ptr<IPdfEngine> engine;
        PageViewer* pageViewer = nullptr;
        ThumbnailPanel* thumbPanel = nullptr;
    };

    QList<DocContext> m_docs;

    DocContext* activeContext();
    const DocContext* activeContext() const;

    // UI components
    QSplitter*         m_mainSplitter     = nullptr;
    SidebarMenu*       m_sidebarMenu      = nullptr;
    CollapsiblePanel*  m_collapsiblePanel = nullptr;
    QTabWidget*        m_tabWidget        = nullptr;
    QProgressBar*      m_progressBar      = nullptr;
    QLabel*            m_statusLabel      = nullptr;
    QLabel*            m_noDocumentPanel  = nullptr;
    QWidget*           m_toolsPanel       = nullptr;

    // Actions - File
    QAction* m_actOpen         = nullptr;
    QAction* m_actSaveEditedAs = nullptr;
    QAction* m_actClose        = nullptr;
    QAction* m_actQuit         = nullptr;

    // Actions - Edit
    QAction* m_actMerge   = nullptr;
    QAction* m_actSplit   = nullptr;

    // Actions - Tools
    QAction* m_actCompress      = nullptr;
    QAction* m_actExtractText   = nullptr;
    QAction* m_actEncrypt       = nullptr;
    QAction* m_actDecrypt       = nullptr;

    // Actions - Page
    QAction* m_actRotateLeft  = nullptr;
    QAction* m_actRotateRight = nullptr;
    QAction* m_actExportImgs  = nullptr;
    QAction* m_actImportImgs  = nullptr;

    // Actions - View
    QAction* m_actZoomIn    = nullptr;
    QAction* m_actZoomOut   = nullptr;
    QAction* m_actFitWidth  = nullptr;

    // Page clipboard (cross-tab copy/cut/paste support)
    QString m_pageClipboardFile;
    bool m_pageClipboardWasCut = false;
};


