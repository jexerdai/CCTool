#include "LeftPanel.h"
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

LeftPanel::LeftPanel(QWidget* parent) : QWidget(parent)
{
    setObjectName("LeftPanel");
    setFixedWidth(220);
    setupUi();
}

void LeftPanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_list = new QListWidget(this);
    m_list->setObjectName("RepoList");
    layout->addWidget(m_list);

    // 底部工具栏
    auto* toolbar = new QWidget(this);
    toolbar->setObjectName("RepoToolBar");
    auto* toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setContentsMargins(6, 4, 6, 4);
    toolLayout->setSpacing(4);

    m_addBtn = new QPushButton("+ 添加", toolbar);
    m_addBtn->setObjectName("AddRepoBtn");
    m_removeBtn = new QPushButton("删除", toolbar);
    m_removeBtn->setObjectName("RemoveRepoBtn");

    toolLayout->addWidget(m_addBtn);
    toolLayout->addWidget(m_removeBtn);
    layout->addWidget(toolbar);

    connect(m_addBtn,    &QPushButton::clicked, this, &LeftPanel::onAddClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &LeftPanel::onRemoveClicked);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &LeftPanel::onItemDoubleClicked);
}

void LeftPanel::addRepo(const RepoInfo& repo)
{
    auto* item = new QListWidgetItem(m_list);
    item->setText(repo.name + "\n" + QDir::toNativeSeparators(repo.path));
    item->setData(Qt::UserRole, QVariant::fromValue(repo.id));
    item->setData(Qt::UserRole + 1, QVariant::fromValue(repo));
    item->setToolTip(repo.path);
}

void LeftPanel::removeCurrentRepo()
{
    auto* item = m_list->currentItem();
    if (!item) return;
    delete item;
}

void LeftPanel::loadRepos(const QList<RepoInfo>& repos)
{
    m_list->clear();
    for (const auto& r : repos)
        addRepo(r);
}

void LeftPanel::onAddClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "选择仓库目录", QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return;

    RepoInfo repo;
    repo.name = QDir(dir).dirName();
    repo.path = dir;
    emit repoOpenRequested(repo);
}

void LeftPanel::onRemoveClicked()
{
    auto* item = m_list->currentItem();
    if (!item) return;

    int repoId = item->data(Qt::UserRole).toInt();
    if (QMessageBox::question(this, "删除仓库",
            "确认从列表中移除该仓库？（不会删除本地文件）",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        emit repoRemoveRequested(repoId);
        delete item;
    }
}

void LeftPanel::onItemDoubleClicked(QListWidgetItem* item)
{
    if (!item) return;
    auto repo = item->data(Qt::UserRole + 1).value<RepoInfo>();
    emit repoOpenRequested(repo);
}
