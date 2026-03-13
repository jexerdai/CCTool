#include "SessionPickerDialog.h"
#include <QListWidget>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>

SessionPickerDialog::SessionPickerDialog(const QList<CCSession>& sessions,
                                         const QString& lastSessionId,
                                         QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("选择 Session");
    setMinimumWidth(480);

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(12, 12, 12, 12);

    auto* label = new QLabel("请选择要继续的对话 Session：", this);
    layout->addWidget(label);

    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(m_list);

    int defaultRow = 0;
    for (int i = 0; i < sessions.size(); ++i) {
        const CCSession& s = sessions[i];
        QString timeStr = s.createdAt.toLocalTime().toString("yyyy-MM-dd hh:mm");
        QString preview = s.firstMessage.isEmpty() ? "(无消息)" : s.firstMessage;
        auto* item = new QListWidgetItem(timeStr + "  " + preview, m_list);
        item->setData(Qt::UserRole, s.sessionId);
        item->setToolTip(s.sessionId);
        if (s.sessionId == lastSessionId)
            defaultRow = i;
    }

    if (m_list->count() > 0) {
        m_list->setCurrentRow(defaultRow);
        m_list->scrollToItem(m_list->currentItem());
    }

    // 双击直接确认
    connect(m_list, &QListWidget::itemDoubleClicked, this, &QDialog::accept);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString SessionPickerDialog::selectedSessionId() const
{
    QListWidgetItem* item = m_list->currentItem();
    if (!item) return {};
    return item->data(Qt::UserRole).toString();
}
