#include "MainWindow.h"

#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QSplitter>
#include <QToolButton>
#include <QTimer>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStandardPaths>
#include <QUuid>
#include <QMimeData>
#include <QUrl>
#include <QSet>
#include <algorithm>

#include "engine/PdfiumEngine.h"
#include "operations/PdfOperations.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("PDFKill");
    setAcceptDrops(true);

    setupMenuBar();
    setupCentralWidget();
    setupSidebar();
    setupStatusBar();

    setOperationsEnabled(false);
}

MainWindow::~MainWindow()
{
    for (DocContext& ctx : m_docs) {
        cleanupWorkingCopy(ctx);
    }
    if (!m_pageClipboardFile.isEmpty()) {
        QFile::remove(m_pageClipboardFile);
        m_pageClipboardFile.clear();
    }
}

MainWindow::DocContext* MainWindow::activeContext()
{
    if (m_docs.isEmpty() || !m_tabWidget || m_tabWidget->currentIndex() < 0) {
        return nullptr;
    }
    return &m_docs[m_tabWidget->currentIndex()];
}

const MainWindow::DocContext* MainWindow::activeContext() const
{
    if (m_docs.isEmpty() || !m_tabWidget || m_tabWidget->currentIndex() < 0) {
        return nullptr;
    }
    return &m_docs[m_tabWidget->currentIndex()];
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (!event || !event->mimeData() || !event->mimeData()->hasUrls()) {
        return;
    }

    for (const QUrl& url : event->mimeData()->urls()) {
        const QString path = url.toLocalFile();
        if (!path.isEmpty() && path.endsWith(".pdf", Qt::CaseInsensitive)) {
            event->acceptProposedAction();
            return;
        }
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if (!event || !event->mimeData() || !event->mimeData()->hasUrls()) {
        return;
    }

    bool openedAny = false;
    for (const QUrl& url : event->mimeData()->urls()) {
        const QString path = url.toLocalFile();
        if (!path.isEmpty() && path.endsWith(".pdf", Qt::CaseInsensitive)) {
            loadDocument(path);
            openedAny = true;
        }
    }

    if (openedAny) {
        event->acceptProposedAction();
    }
}


void MainWindow::setupCentralWidget()
{
    auto* mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);

    auto* rootLayout = new QHBoxLayout(mainWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_sidebarMenu = new SidebarMenu(this);
    rootLayout->addWidget(m_sidebarMenu);

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    rootLayout->addWidget(m_mainSplitter, 1);

    m_collapsiblePanel = new CollapsiblePanel(this);
    m_collapsiblePanel->hide();
    m_mainSplitter->addWidget(m_collapsiblePanel);

    m_noDocumentPanel = new QLabel(tr("No document"), m_collapsiblePanel);
    m_noDocumentPanel->setAlignment(Qt::AlignCenter);
    m_noDocumentPanel->setStyleSheet("background-color: #E6E6E6; color: #111111; padding: 8px;");
    m_noDocumentPanel->hide();

    m_toolsPanel = new QWidget(m_collapsiblePanel);
    auto* toolsLayout = new QVBoxLayout(m_toolsPanel);
    toolsLayout->setContentsMargins(10, 10, 10, 10);
    toolsLayout->setSpacing(8);

    auto* btnMerge = new QPushButton(tr("Merge PDFs"), m_toolsPanel);
    auto* btnSplit = new QPushButton(tr("Split PDF"), m_toolsPanel);
    btnMerge->setEnabled(m_actMerge->isEnabled());
    btnSplit->setEnabled(m_actSplit->isEnabled());
    connect(m_actMerge, &QAction::enabledChanged, btnMerge, &QPushButton::setEnabled);
    connect(m_actSplit, &QAction::enabledChanged, btnSplit, &QPushButton::setEnabled);
    connect(btnMerge, &QPushButton::clicked, m_actMerge, &QAction::trigger);
    connect(btnSplit, &QPushButton::clicked, m_actSplit, &QAction::trigger);

    toolsLayout->addWidget(btnMerge);
    toolsLayout->addWidget(btnSplit);
    toolsLayout->addStretch();

    m_toolsPanel->setStyleSheet("background-color: #E6E6E6; color: #111111; padding: 8px;");
    m_toolsPanel->hide();

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setStyleSheet("QTabBar::tab { padding: 8px 16px; background: #2D2D2D; color: #AAA; border-right: 1px solid #111; } QTabBar::tab:selected { background: #4E148C; color: white; font-weight: bold; } QTabWidget::pane { border: none; }");

    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);

    m_mainSplitter->addWidget(m_tabWidget);
    m_mainSplitter->setSizes({130, 2000});
    m_mainSplitter->setCollapsible(0, true);
}

void MainWindow::setupMenuBar()
{
    menuBar()->setStyleSheet(
        "QMenuBar {"
        "  background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "    stop:0 rgba(120,8,8,255), stop:0.45 rgba(214,36,36,255), stop:1 rgba(128,10,10,255));"
        "  color: #FFFFFF;"
        "  border-top: 1px solid rgba(255,255,255,0.40);"
        "  border-bottom: 1px solid rgba(60,0,0,0.90);"
        "}"
        "QMenuBar::item { background: transparent; padding: 6px 12px; }"
        "QMenuBar::item:selected { background-color: rgba(255,255,255,0.26); }"
        "QMenuBar::item:pressed { background-color: rgba(255,255,255,0.34); }"
        "QMenu { background-color: #2A2A2A; color: #FFFFFF; }"
        "QMenu::item:selected { background-color: #A12626; }");

    auto* fileMenu = menuBar()->addMenu(tr("&File"));

    m_actOpen         = fileMenu->addAction(tr("&Open..."),        this, &MainWindow::onOpenFile, QKeySequence::Open);
    m_actSaveEditedAs = fileMenu->addAction(tr("Save &Edited As..."), this, &MainWindow::onSaveEditedAs);
    m_actClose        = fileMenu->addAction(tr("&Close"),          this, &MainWindow::onCloseFile);
    fileMenu->addSeparator();
    m_actQuit         = fileMenu->addAction(tr("&Quit"),           qApp, &QApplication::quit, QKeySequence::Quit);

    auto* editMenu = menuBar()->addMenu(tr("&Edit"));
    m_actMerge = editMenu->addAction(tr("&Merge PDFs..."), this, &MainWindow::onMerge);
    m_actSplit = editMenu->addAction(tr("&Split PDF..."), this, &MainWindow::onSplit);

    auto* toolsMenu = menuBar()->addMenu(tr("&Tools"));
    m_actCompress    = toolsMenu->addAction(tr("&Compress / Optimize..."), this, &MainWindow::onCompress);
    m_actExtractText = toolsMenu->addAction(tr("&Extract Text..."), this, &MainWindow::onExtractText);
    toolsMenu->addSeparator();
    m_actEncrypt = toolsMenu->addAction(tr("&Encrypt..."), this, &MainWindow::onEncrypt);
    m_actDecrypt = toolsMenu->addAction(tr("&Decrypt..."), this, &MainWindow::onDecrypt);

    auto* pageMenu = menuBar()->addMenu(tr("&Page"));
    m_actRotateLeft = pageMenu->addAction(tr("Rotate &Left"), this, &MainWindow::onRotateLeft);
    m_actRotateRight = pageMenu->addAction(tr("Rotate &Right"), this, &MainWindow::onRotateRight);
    pageMenu->addSeparator();
    m_actExportImgs = pageMenu->addAction(tr("Export Pages to Images..."), this, &MainWindow::onExportToImages);
    m_actImportImgs = pageMenu->addAction(tr("Import Images to PDF..."), this, &MainWindow::onImportFromImages);

    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    m_actZoomIn = viewMenu->addAction(tr("Zoom &In"), this, &MainWindow::onZoomIn, QKeySequence::ZoomIn);
    m_actZoomOut = viewMenu->addAction(tr("Zoom &Out"), this, &MainWindow::onZoomOut, QKeySequence::ZoomOut);
    m_actFitWidth = viewMenu->addAction(tr("&Fit Width"), this, &MainWindow::onFitWidth, tr("Ctrl+W"));
}

void MainWindow::setupToolBar()
{
    // Intentionally disabled: toolbar strip below menu bar removed.
}


void MainWindow::setupSidebar()
{
    connect(m_sidebarMenu, &SidebarMenu::pagesClicked, this, [this]() {
        if (m_collapsiblePanel->isVisible()) {
            m_collapsiblePanel->hide();
        } else {
            m_collapsiblePanel->setTitle(tr("Pages"));
            if (auto* ctx = activeContext()) {
                m_collapsiblePanel->setCentralWidget(ctx->thumbPanel);
            } else {
                m_collapsiblePanel->setCentralWidget(m_noDocumentPanel);
            }
            m_collapsiblePanel->show();
        }
    });

    connect(m_sidebarMenu, &SidebarMenu::toolsClicked, this, [this]() {
        if (m_collapsiblePanel->isVisible()) {
            m_collapsiblePanel->hide();
        } else {
            m_collapsiblePanel->setTitle(tr("Tools"));
            m_collapsiblePanel->setCentralWidget(m_toolsPanel);
            m_collapsiblePanel->show();
        }
    });
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel(tr("No document open"), this);
    m_statusLabel->setStyleSheet("color: #FFFFFF; background: transparent;");
    m_progressBar = new QProgressBar(this);
    m_progressBar->setFixedWidth(180);
    m_progressBar->setVisible(false);

    statusBar()->setStyleSheet(
        "QStatusBar {"
        "  background-color: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "    stop:0 rgba(120,8,8,255), stop:0.45 rgba(214,36,36,255), stop:1 rgba(128,10,10,255));"
        "  color: #FFFFFF;"
        "  border-top: 1px solid rgba(255,255,255,0.32);"
        "}"
        "QStatusBar::item { border: none; }");

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_progressBar);
}

QString MainWindow::activeDocumentPath(const MainWindow::DocContext* ctx) const
{
    if (!ctx) {
        return QString();
    }
    return ctx->workingPath.isEmpty() ? ctx->sourcePath : ctx->workingPath;
}

bool MainWindow::ensureWorkingCopy(MainWindow::DocContext* ctx)
{
    if (!ctx) {
        return false;
    }
    if (!ctx->workingPath.isEmpty()) {
        return true;
    }
    if (ctx->sourcePath.isEmpty()) {
        return false;
    }

    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempDir.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Unable to create temporary working file."));
        return false;
    }

    const QString tempPath = QDir(tempDir).filePath(
        QString("pdfkill-edit-%1.pdf").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));

    if (!QFile::copy(ctx->sourcePath, tempPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create editable copy:\n%1").arg(ctx->sourcePath));
        return false;
    }

    ctx->workingPath = tempPath;
    return true;
}

bool MainWindow::saveEditedAs(MainWindow::DocContext* ctx, const QString& outPath)
{
    if (!ctx || outPath.isEmpty()) {
        return false;
    }

    const QString inPath = activeDocumentPath(ctx);
    if (inPath.isEmpty()) {
        return false;
    }

    if (QFile::exists(outPath) && !QFile::remove(outPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot overwrite output file:\n%1").arg(outPath));
        return false;
    }

    if (!QFile::copy(inPath, outPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Save Edited As failed:\n%1").arg(outPath));
        return false;
    }

    ctx->exportPath = outPath;
    ctx->dirty = false;
    return true;
}

bool MainWindow::maybeSaveEdits(MainWindow::DocContext* ctx)
{
    if (!ctx || !ctx->dirty) {
        return true;
    }

    QMessageBox::StandardButton choice = QMessageBox::question(
        this,
        tr("Unsaved edits"),
        tr("This document has unsaved edits. Save an edited copy before closing?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Yes);

    if (choice == QMessageBox::Cancel) {
        return false;
    }

    if (choice == QMessageBox::No) {
        return true;
    }

    const QString defaultName = QFileInfo(ctx->sourcePath).completeBaseName() + "_edited.pdf";
    const QString outPath = QFileDialog::getSaveFileName(
        this, tr("Save edited PDF"), defaultName, tr("PDF Files (*.pdf)"));
    if (outPath.isEmpty()) {
        return false;
    }

    return saveEditedAs(ctx, outPath);
}

void MainWindow::cleanupWorkingCopy(MainWindow::DocContext& ctx)
{
    if (!ctx.workingPath.isEmpty()) {
        QFile::remove(ctx.workingPath);
        ctx.workingPath.clear();
    }
    ctx.dirty = false;
}

void MainWindow::loadDocument(const QString& filePath)
{
    auto engine = std::make_shared<PdfiumEngine>();
    if (!engine->load(filePath)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open:\n%1").arg(filePath));
        return;
    }

    auto* pv = new PageViewer(this);
    auto* tp = new ThumbnailPanel(this);

    connect(tp, &ThumbnailPanel::pageSelected, pv, &PageViewer::showPage);
    connect(tp, &ThumbnailPanel::pagesReordered, this, &MainWindow::onThumbPagesReordered);
    connect(tp, &ThumbnailPanel::rotateLeftRequested, this, &MainWindow::onThumbRotateLeft);
    connect(tp, &ThumbnailPanel::rotateRightRequested, this, &MainWindow::onThumbRotateRight);
    connect(tp, &ThumbnailPanel::deleteRequested, this, &MainWindow::onThumbDelete);
    connect(tp, &ThumbnailPanel::copyRequested, this, &MainWindow::onThumbCopyRequested);
    connect(tp, &ThumbnailPanel::cutRequested, this, &MainWindow::onThumbCutRequested);
    connect(tp, &ThumbnailPanel::pasteRequested, this, &MainWindow::onThumbPasteRequested);
    connect(pv, &PageViewer::pageChanged, this, &MainWindow::onPageChanged);
    connect(pv, &PageViewer::rotateLeftRequested, this, &MainWindow::onRotateLeft);
    connect(pv, &PageViewer::rotateRightRequested, this, &MainWindow::onRotateRight);

    pv->setEngine(engine.get());
    tp->loadFile(filePath, engine->pageCount());

    DocContext ctx;
    ctx.sourcePath = filePath;
    ctx.engine = engine;
    ctx.pageViewer = pv;
    ctx.thumbPanel = tp;

    m_docs.append(ctx);
    int index = m_tabWidget->addTab(pv, QFileInfo(filePath).fileName());
    m_tabWidget->setCurrentIndex(index);

    pv->showPage(0);
    updateWindowTitle();

    QTimer::singleShot(20, this, [pv]() {
        if (pv) {
            pv->fitWidth();
        }
    });
}

void MainWindow::onTabChanged(int)
{
    DocContext* ctx = activeContext();
    if (!ctx) {
        updateOperationsState();
        updateWindowTitle();
        if (m_collapsiblePanel->isVisible()) {
            m_collapsiblePanel->setCentralWidget(m_noDocumentPanel);
        }
        return;
    }

    updateOperationsState();
    updateWindowTitle();

    if (m_collapsiblePanel->isVisible() &&
        dynamic_cast<ThumbnailPanel*>(m_collapsiblePanel->currentCentralWidget())) {
        m_collapsiblePanel->setCentralWidget(ctx->thumbPanel);
    }
}

void MainWindow::onTabCloseRequested(int index)
{
    if (index < 0 || index >= m_docs.size()) {
        return;
    }

    DocContext& ctx = m_docs[index];
    if (!maybeSaveEdits(&ctx)) {
        return;
    }

    m_tabWidget->removeTab(index);

    if (ctx.thumbPanel) {
        ctx.thumbPanel->deleteLater();
    }
    if (ctx.pageViewer) {
        ctx.pageViewer->deleteLater();
    }
    if (ctx.engine) {
        ctx.engine->close();
    }
    cleanupWorkingCopy(ctx);

    m_docs.removeAt(index);
    updateOperationsState();
    updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
    const DocContext* ctx = activeContext();
    if (ctx) {
        const QString fileName = QFileInfo(ctx->sourcePath).fileName();
        const QString dirtyMark = ctx->dirty ? QStringLiteral("*") : QString();
        setWindowTitle(QString("PDFKill - %1%2").arg(fileName, dirtyMark));

        const int page = ctx->pageViewer ? ctx->pageViewer->currentPage() : 0;
        const int total = ctx->engine ? ctx->engine->pageCount() : 0;
        if (total > 0) {
            m_statusLabel->setText(tr("Page %1 of %2").arg(page + 1).arg(total));
        } else {
            m_statusLabel->setText(tr("No document open"));
        }

        const int idx = m_tabWidget ? m_tabWidget->currentIndex() : -1;
        if (idx >= 0) {
            m_tabWidget->setTabText(idx, QString("%1%2").arg(fileName, dirtyMark));
        }
    } else {
        setWindowTitle("PDFKill");
        m_statusLabel->setText(tr("No document open"));
    }
}

void MainWindow::updateOperationsState()
{
    const DocContext* ctx = activeContext();
    const bool enabled = (ctx != nullptr && ctx->engine && ctx->engine->isOpen());
    setOperationsEnabled(enabled);
}

void MainWindow::setOperationsEnabled(bool enabled)
{
    for (auto* act : {m_actSaveEditedAs, m_actClose, m_actMerge, m_actSplit,
                      m_actCompress, m_actExtractText,
                      m_actEncrypt, m_actDecrypt,
                      m_actRotateLeft, m_actRotateRight,
                      m_actExportImgs,
                      m_actZoomIn, m_actZoomOut, m_actFitWidth}) {
        if (act) {
            act->setEnabled(enabled);
        }
    }
}

void MainWindow::onOpenFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open PDF"), QString(), tr("PDF Files (*.pdf);;All Files (*)"));

    if (!path.isEmpty()) {
        loadDocument(path);
    }
}

void MainWindow::onSaveEditedAs()
{
    DocContext* ctx = activeContext();
    if (!ctx) {
        return;
    }

    const QString defaultName = QFileInfo(ctx->sourcePath).completeBaseName() + "_edited.pdf";
    const QString outPath = QFileDialog::getSaveFileName(
        this, tr("Save edited PDF"), defaultName, tr("PDF Files (*.pdf)"));
    if (outPath.isEmpty()) {
        return;
    }

    if (saveEditedAs(ctx, outPath)) {
        QMessageBox::information(this, tr("Done"), tr("Edited copy saved to:\n%1").arg(outPath));
        updateWindowTitle();
    }
}

void MainWindow::onCloseFile()
{
    if (m_tabWidget) {
        onTabCloseRequested(m_tabWidget->currentIndex());
    }
}

void MainWindow::onMerge()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this, tr("Select PDFs to merge"), QString(), tr("PDF Files (*.pdf)"));
    if (files.isEmpty()) {
        return;
    }

    const QString out = QFileDialog::getSaveFileName(
        this, tr("Save merged PDF"), QString(), tr("PDF Files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }

    PdfOperations ops;
    if (ops.merge(files, out)) {
        QMessageBox::information(this, tr("Done"), tr("PDFs merged successfully."));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Merge failed: %1").arg(ops.lastError()));
    }
}

void MainWindow::onSplit()
{
    DocContext* ctx = activeContext();
    const QString inPath = activeDocumentPath(ctx);
    if (inPath.isEmpty()) {
        return;
    }

    const QString outDir = QFileDialog::getExistingDirectory(this, tr("Select output directory"));
    if (outDir.isEmpty()) {
        return;
    }

    PdfOperations ops;
    if (ops.split(inPath, outDir)) {
        QMessageBox::information(this, tr("Done"), tr("Pages split into:\n%1").arg(outDir));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Split failed: %1").arg(ops.lastError()));
    }
}

void MainWindow::onCompress()
{
    DocContext* ctx = activeContext();
    const QString inPath = activeDocumentPath(ctx);
    if (inPath.isEmpty()) {
        return;
    }

    const QString out = QFileDialog::getSaveFileName(
        this, tr("Save compressed PDF"), QString(), tr("PDF Files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }

    PdfOperations ops;
    if (ops.compress(inPath, out)) {
        QMessageBox::information(this, tr("Done"), tr("PDF compressed successfully."));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Compress failed: %1").arg(ops.lastError()));
    }
}

void MainWindow::onExtractText()
{
    DocContext* ctx = activeContext();
    const QString inPath = activeDocumentPath(ctx);
    if (inPath.isEmpty()) {
        return;
    }

    const QString out = QFileDialog::getSaveFileName(
        this, tr("Save extracted text"), QString(), tr("Text Files (*.txt)"));
    if (out.isEmpty()) {
        return;
    }

    PdfOperations ops;
    if (ops.extractText(inPath, out)) {
        QMessageBox::information(this, tr("Done"), tr("Text extracted to:\n%1").arg(out));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Extraction failed: %1").arg(ops.lastError()));
    }
}

void MainWindow::onEncrypt()
{
    DocContext* ctx = activeContext();
    const QString inPath = activeDocumentPath(ctx);
    if (inPath.isEmpty()) {
        return;
    }

    bool ok = false;
    const QString userPassword = QInputDialog::getText(
        this, tr("Encrypt PDF"), tr("User password:"), QLineEdit::Password, QString(), &ok);
    if (!ok) {
        return;
    }

    const QString ownerPassword = QInputDialog::getText(
        this, tr("Encrypt PDF"), tr("Owner password:"), QLineEdit::Password, QString(), &ok);
    if (!ok) {
        return;
    }

    const QString out = QFileDialog::getSaveFileName(
        this, tr("Save encrypted PDF"), QString(), tr("PDF Files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }

    PdfOperations ops;
    if (ops.encrypt(inPath, out, userPassword, ownerPassword)) {
        QMessageBox::information(this, tr("Done"), tr("PDF encrypted successfully."));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Encrypt failed: %1").arg(ops.lastError()));
    }
}

void MainWindow::onDecrypt()
{
    DocContext* ctx = activeContext();
    const QString inPath = activeDocumentPath(ctx);
    if (inPath.isEmpty()) {
        return;
    }

    bool ok = false;
    const QString password = QInputDialog::getText(
        this, tr("Decrypt PDF"), tr("Password:"), QLineEdit::Password, QString(), &ok);
    if (!ok) {
        return;
    }

    const QString out = QFileDialog::getSaveFileName(
        this, tr("Save decrypted PDF"), QString(), tr("PDF Files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }

    PdfOperations ops;
    if (ops.decrypt(inPath, out, password)) {
        QMessageBox::information(this, tr("Done"), tr("PDF decrypted successfully."));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Decrypt failed: %1").arg(ops.lastError()));
    }
}

void MainWindow::onRotateLeft()
{
    if (auto* ctx = activeContext()) {
        rotatePageAt(ctx->pageViewer ? ctx->pageViewer->currentPage() : 0, -90);
    }
}

void MainWindow::onRotateRight()
{
    if (auto* ctx = activeContext()) {
        rotatePageAt(ctx->pageViewer ? ctx->pageViewer->currentPage() : 0, 90);
    }
}

void MainWindow::onExportToImages()
{
    DocContext* ctx = activeContext();
    const QString inPath = activeDocumentPath(ctx);
    if (!ctx || inPath.isEmpty() || !ctx->engine) {
        return;
    }

    const QString outDir = QFileDialog::getExistingDirectory(
        this, tr("Select output directory for images"));
    if (outDir.isEmpty()) {
        return;
    }

    PdfOperations ops;
    if (ops.exportToImages(inPath, outDir, ctx->engine.get())) {
        QMessageBox::information(this, tr("Done"), tr("Pages exported to:\n%1").arg(outDir));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Export failed: %1").arg(ops.lastError()));
    }
}

void MainWindow::onImportFromImages()
{
    QStringList images = QFileDialog::getOpenFileNames(
        this, tr("Select images"), QString(), tr("Images (*.png *.jpg *.jpeg *.bmp *.tiff)"));
    if (images.isEmpty()) {
        return;
    }

    const QString out = QFileDialog::getSaveFileName(
        this, tr("Save output PDF"), QString(), tr("PDF Files (*.pdf)"));
    if (out.isEmpty()) {
        return;
    }

    PdfOperations ops;
    if (ops.importFromImages(images, out)) {
        QMessageBox::information(this, tr("Done"), tr("PDF created from images."));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Import failed: %1").arg(ops.lastError()));
    }
}

void MainWindow::onPageChanged(int pageIndex)
{
    DocContext* ctx = activeContext();
    if (!ctx || !ctx->engine || !ctx->thumbPanel) {
        return;
    }

    m_statusLabel->setText(tr("Page %1 of %2").arg(pageIndex + 1).arg(ctx->engine->pageCount()));
    ctx->thumbPanel->selectPage(pageIndex);
}


bool MainWindow::createNextWorkingCopy(DocContext* ctx, QString& prevPath, QString& nextPath)
{
    if (!ctx || !ensureWorkingCopy(ctx)) {
        return false;
    }

    prevPath = ctx->workingPath;
    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    nextPath = QDir(tempDir).filePath(
        QString("pdfkill-edit-%1.pdf").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));

    if (!QFile::copy(prevPath, nextPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Edit failed: could not create editable copy."));
        return false;
    }

    return true;
}

bool MainWindow::applyEditedPath(DocContext* ctx, const QString& prevPath, const QString& nextPath, int pageToShow)
{
    if (!ctx || !ctx->engine || !ctx->thumbPanel || !ctx->pageViewer) {
        return false;
    }

    if (!ctx->engine->load(nextPath)) {
        QFile::remove(nextPath);
        QMessageBox::critical(this, tr("Error"), tr("Failed to reload document after edit."));
        return false;
    }

    ctx->thumbPanel->clear();
    ctx->workingPath = nextPath;
    ctx->dirty = true;
    ctx->thumbPanel->loadFile(nextPath, ctx->engine->pageCount());
    ctx->pageViewer->refresh();
    ctx->pageViewer->showPage(qBound(0, pageToShow, qMax(0, ctx->engine->pageCount() - 1)));

    if (!prevPath.isEmpty() && prevPath != nextPath) {
        QFile::remove(prevPath);
    }

    updateWindowTitle();
    return true;
}

void MainWindow::rotatePageAt(int pageIndex, int deltaDegrees)
{
    DocContext* ctx = activeContext();
    if (!ctx || !ctx->engine || !ctx->thumbPanel || !ctx->pageViewer) {
        return;
    }

    QString prevPath;
    QString nextPath;
    if (!createNextWorkingCopy(ctx, prevPath, nextPath)) {
        return;
    }

    PdfOperations ops;
    if (!ops.rotatePage(nextPath, pageIndex, deltaDegrees)) {
        QFile::remove(nextPath);
        QMessageBox::critical(this, tr("Error"), tr("Rotate failed: %1").arg(ops.lastError()));
        return;
    }

    applyEditedPath(ctx, prevPath, nextPath, pageIndex);
}

void MainWindow::deletePageAt(int pageIndex)
{
    DocContext* ctx = activeContext();
    if (!ctx || !ctx->engine || !ctx->thumbPanel || !ctx->pageViewer) {
        return;
    }

    QString prevPath;
    QString nextPath;
    if (!createNextWorkingCopy(ctx, prevPath, nextPath)) {
        return;
    }

    PdfOperations ops;
    if (!ops.deletePage(nextPath, pageIndex)) {
        QFile::remove(nextPath);
        QMessageBox::critical(this, tr("Error"), tr("Delete page failed: %1").arg(ops.lastError()));
        return;
    }

    applyEditedPath(ctx, prevPath, nextPath, pageIndex);
}

void MainWindow::reorderPagesTo(const QList<int>& orderedPageIndices)
{
    DocContext* ctx = activeContext();
    if (!ctx || !ctx->engine || !ctx->thumbPanel || !ctx->pageViewer) {
        return;
    }

    QString prevPath;
    QString nextPath;
    if (!createNextWorkingCopy(ctx, prevPath, nextPath)) {
        return;
    }

    PdfOperations ops;
    if (!ops.reorderPages(nextPath, orderedPageIndices)) {
        QFile::remove(nextPath);
        QMessageBox::critical(this, tr("Error"), tr("Reorder pages failed: %1").arg(ops.lastError()));
        return;
    }

    applyEditedPath(ctx, prevPath, nextPath, ctx->pageViewer->currentPage());
}

void MainWindow::onThumbPagesReordered(const QList<int>& orderedPageIndices)
{
    if (orderedPageIndices.isEmpty()) {
        return;
    }
    reorderPagesTo(orderedPageIndices);
}

void MainWindow::onThumbRotateLeft(int pageIndex)
{
    rotatePageAt(pageIndex, -90);
}

void MainWindow::onThumbRotateRight(int pageIndex)
{
    rotatePageAt(pageIndex, 90);
}

void MainWindow::onThumbDelete(int pageIndex)
{
    deletePageAt(pageIndex);
}

QList<int> MainWindow::normalizePageIndices(const QList<int>& pageIndices, int totalPages) const
{
    QList<int> normalized;
    QSet<int> seen;
    for (int idx : pageIndices) {
        if (idx < 0 || idx >= totalPages || seen.contains(idx)) {
            continue;
        }
        seen.insert(idx);
        normalized.append(idx);
    }
    std::sort(normalized.begin(), normalized.end());
    return normalized;
}

void MainWindow::onThumbCopyRequested(const QList<int>& pageIndices)
{
    DocContext* ctx = activeContext();
    if (!ctx || !ctx->engine) {
        return;
    }

    const QList<int> pages = normalizePageIndices(pageIndices, ctx->engine->pageCount());
    if (pages.isEmpty()) {
        return;
    }

    const QString srcPath = activeDocumentPath(ctx);
    if (srcPath.isEmpty()) {
        return;
    }

    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempDir.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Unable to access temporary location for page clipboard."));
        return;
    }

    const QString clipPath = QDir(tempDir).filePath(
        QString("pdfkill-pages-clipboard-%1.pdf").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));

    PdfOperations ops;
    if (!ops.extractPages(srcPath, pages, clipPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Copy pages failed: %1").arg(ops.lastError()));
        return;
    }

    if (!m_pageClipboardFile.isEmpty() && m_pageClipboardFile != clipPath) {
        QFile::remove(m_pageClipboardFile);
    }

    m_pageClipboardFile = clipPath;
    m_pageClipboardWasCut = false;
}

void MainWindow::onThumbCutRequested(const QList<int>& pageIndices)
{
    DocContext* ctx = activeContext();
    if (!ctx || !ctx->engine || !ctx->thumbPanel || !ctx->pageViewer) {
        return;
    }

    const QList<int> pages = normalizePageIndices(pageIndices, ctx->engine->pageCount());
    if (pages.isEmpty()) {
        return;
    }

    const QString srcPath = activeDocumentPath(ctx);
    if (srcPath.isEmpty()) {
        return;
    }

    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempDir.isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Unable to access temporary location for page clipboard."));
        return;
    }

    const QString clipPath = QDir(tempDir).filePath(
        QString("pdfkill-pages-clipboard-%1.pdf").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));

    PdfOperations ops;
    if (!ops.extractPages(srcPath, pages, clipPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Cut pages failed: %1").arg(ops.lastError()));
        return;
    }

    QString prevPath;
    QString nextPath;
    if (!createNextWorkingCopy(ctx, prevPath, nextPath)) {
        QFile::remove(clipPath);
        return;
    }

    if (!ops.deletePages(nextPath, pages)) {
        QFile::remove(nextPath);
        QFile::remove(clipPath);
        QMessageBox::critical(this, tr("Error"), tr("Cut pages failed: %1").arg(ops.lastError()));
        return;
    }

    const int focusPage = pages.isEmpty() ? 0 : pages.first();
    if (!applyEditedPath(ctx, prevPath, nextPath, focusPage)) {
        QFile::remove(clipPath);
        return;
    }

    if (!m_pageClipboardFile.isEmpty() && m_pageClipboardFile != clipPath) {
        QFile::remove(m_pageClipboardFile);
    }

    m_pageClipboardFile = clipPath;
    m_pageClipboardWasCut = true;
}

void MainWindow::onThumbPasteRequested(int insertAfterPageIndex)
{
    DocContext* ctx = activeContext();
    if (!ctx || !ctx->engine || !ctx->thumbPanel || !ctx->pageViewer) {
        return;
    }

    if (m_pageClipboardFile.isEmpty() || !QFile::exists(m_pageClipboardFile)) {
        QMessageBox::information(this, tr("Paste Pages"), tr("Page clipboard is empty."));
        return;
    }

    const int total = ctx->engine->pageCount();
    const int insertIndex = qBound(0, insertAfterPageIndex + 1, total);

    QString prevPath;
    QString nextPath;
    if (!createNextWorkingCopy(ctx, prevPath, nextPath)) {
        return;
    }

    PdfOperations ops;
    if (!ops.insertPages(nextPath, m_pageClipboardFile, insertIndex)) {
        QFile::remove(nextPath);
        QMessageBox::critical(this, tr("Error"), tr("Paste pages failed: %1").arg(ops.lastError()));
        return;
    }

    if (!applyEditedPath(ctx, prevPath, nextPath, insertIndex)) {
        return;
    }

    if (m_pageClipboardWasCut) {
        QFile::remove(m_pageClipboardFile);
        m_pageClipboardFile.clear();
        m_pageClipboardWasCut = false;
    }
}

void MainWindow::onZoomIn()
{
    if (auto* ctx = activeContext()) {
        ctx->pageViewer->zoom(+0.25f);
    }
}

void MainWindow::onZoomOut()
{
    if (auto* ctx = activeContext()) {
        ctx->pageViewer->zoom(-0.25f);
    }
}

void MainWindow::onFitWidth()
{
    if (auto* ctx = activeContext()) {
        ctx->pageViewer->fitWidth();
    }
}















