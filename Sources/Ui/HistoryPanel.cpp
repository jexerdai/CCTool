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

void HistoryPanel::loadSessions(const QList<CCSession>& sessions)
{
    m_list->clear();
    for (const auto& s : sessions) {
        auto* item = new QListWidgetItem(m_list);
        QString timeStr = s.createdAt.toLocalTime().toString("MM-dd hh:mm");
        QString preview = s.firstMessage.isEmpty() ? "(无消息)" : s.firstMessage;
        item->setText(timeStr + "\n" + preview);
        item->setData(Qt::UserRole, s.sessionId);
        item->setToolTip(s.sessionId);
    }
}

void HistoryPanel::onItemClicked(QListWidgetItem* item)
{
    if (!item) return;
    emit sessionSelected(item->data(Qt::UserRole).toString());
}
