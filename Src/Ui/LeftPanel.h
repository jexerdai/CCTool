#pragma once
#include <QWidget>
#include <QList>
#include "Data/RepoInfo.h"

class QListWidget;
class QListWidgetItem;
class QPushButton;

class LeftPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LeftPanel(QWidget* parent = nullptr);

    void addRepo(const RepoInfo& repo);
    void removeCurrentRepo();
    void loadRepos(const QList<RepoInfo>& repos);

signals:
    void repoOpenRequested(const RepoInfo& repo);
    void repoRemoveRequested(int repoId);

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    QListWidget* m_list;
    QPushButton* m_addBtn;
    QPushButton* m_removeBtn;

    void setupUi();
};
