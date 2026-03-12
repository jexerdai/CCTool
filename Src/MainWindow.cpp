#include "MainWindow.h"
#include "Ui/LeftPanel.h"
#include "Ui/RepoTab.h"
#include "Data/Database.h"

#include <QSplitter>
#include <QTabWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QFile>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("CCTool");
    setMinimumSize(1100, 700);
    resize(1280, 800);

    loadStyleSheet();
    setupUi();
    setupMenuBar();
    loadSavedRepos();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi()
{
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);

    m_leftPanel = new LeftPanel(splitter);

    m_tabWidget = new QTabWidget(splitter);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);

    splitter->addWidget(m_leftPanel);
    splitter->addWidget(m_tabWidget);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);

    connect(m_leftPanel, &LeftPanel::repoOpenRequested,
            this, &MainWindow::onRepoOpenRequested);
    connect(m_leftPanel, &LeftPanel::repoRemoveRequested,
            this, &MainWindow::onRepoRemoveRequested);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::onTabCloseRequested);
}

void MainWindow::setupMenuBar()
{
    auto* fileMenu = menuBar()->addMenu("文件(&F)");

    auto* addAction = fileMenu->addAction("添加仓库(&A)");
    addAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(addAction, &QAction::triggered, m_leftPanel, &LeftPanel::onAddClicked);

    fileMenu->addSeparator();

    auto* quitAction = fileMenu->addAction("退出(&Q)");
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    auto* helpMenu = menuBar()->addMenu("帮助(&H)");
    auto* aboutAction = helpMenu->addAction("关于 CCTool");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "关于 CCTool",
            "<b>CCTool v0.1</b><br>"
            "Claude Code 图形化工作站<br><br>"
            "使用 C++ + Qt 6 开发");
    });
}

void MainWindow::loadStyleSheet()
{
    QFile f(":/Styles/Main.qss");
    if (f.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
    }
}

void MainWindow::loadSavedRepos()
{
    auto repos = Database::instance().listRepos();
    m_leftPanel->loadRepos(repos);
}

void MainWindow::onRepoOpenRequested(const RepoInfo& repoArg)
{
    // 检查是否已打开该 Tab
    if (repoArg.id >= 0) {
        RepoTab* existing = findTabByRepoId(repoArg.id);
        if (existing) {
            m_tabWidget->setCurrentWidget(existing);
            return;
        }
    }

    // 新仓库：先存入数据库
    RepoInfo repo = repoArg;
    if (repo.id < 0) {
        bool added = Database::instance().addRepo(repo);
        if (!added || repo.id < 0) return;
        m_leftPanel->addRepo(repo);
    }

    // 创建 Tab
    auto* tab = new RepoTab(repo, m_tabWidget);
    int idx = m_tabWidget->addTab(tab, repo.name);
    m_tabWidget->setCurrentIndex(idx);
}

void MainWindow::onRepoRemoveRequested(int repoId)
{
    // 关闭对应 Tab
    RepoTab* tab = findTabByRepoId(repoId);
    if (tab) {
        int idx = m_tabWidget->indexOf(tab);
        if (idx >= 0) m_tabWidget->removeTab(idx);
    }
    Database::instance().removeRepo(repoId);
}

void MainWindow::onTabCloseRequested(int index)
{
    QWidget* w = m_tabWidget->widget(index);
    m_tabWidget->removeTab(index);
    delete w;
}

RepoTab* MainWindow::findTabByRepoId(int repoId)
{
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        auto* tab = qobject_cast<RepoTab*>(m_tabWidget->widget(i));
        if (tab && tab->repoInfo().id == repoId)
            return tab;
    }
    return nullptr;
}
