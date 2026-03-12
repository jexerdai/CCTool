#include "HistoryPanel.h"
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>

HistoryPanel::HistoryPanel(QWidget* parent) : QWidget(parent)
{
    setObjectName("HistoryPanel");
    setFixedWidth(200);
    setupUi();
}

void HistoryPanel::setupUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_titleLabel = new QLabel("历史记录", this);
    m_titleLabel->setContentsMargins(10, 8, 10, 8);
    m_titleLabel->setStyleSheet("color:#a6adc8; font-size:11px; font-weight:bold;");
    layout->addWidget(m_titleLabel);

    m_list = new QListWidget(this);
    layout->addWidget(m_list);

    connect(m_list, &QListWidget::itemClicked, this, &HistoryPanel::onItemClicked);
}

void HistoryPanel::loadSessions(const QList<SessionInfo>& sessions)
{
    m_list->clear();
    for (const auto& s : sessions)
        addSession(s);
}

void HistoryPanel::addSession(const SessionInfo& session)
{
    auto* item = new QListWidgetItem(m_list);
    QString label = session.createdAt.toString("MM-dd hh:mm");
    if (label.isEmpty()) label = "新会话";
    item->setText(label);
    item->setData(Qt::UserRole, session.id);
    item->setToolTip("Session: " + session.ccSessionId);
    m_list->insertItem(0, item);   // 最新的排在最上面
}

void HistoryPanel::onItemClicked(QListWidgetItem* item)
{
    if (!item) return;
    emit sessionSelected(item->data(Qt::UserRole).toInt());
}
