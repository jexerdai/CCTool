#pragma once
#include <QMainWindow>
#include "Data/RepoInfo.h"

class LeftPanel;
class QTabWidget;
class RepoTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onRepoOpenRequested(const RepoInfo& repo);
    void onRepoRemoveRequested(int repoId);
    void onTabCloseRequested(int index);

private:
    void setupUi();
    void setupMenuBar();
    void loadStyleSheet();
    void loadSavedRepos();

    RepoTab* findTabByRepoId(int repoId);

    LeftPanel*   m_leftPanel;
    QTabWidget*  m_tabWidget;
};
