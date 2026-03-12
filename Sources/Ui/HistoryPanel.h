#pragma once
#include <QWidget>
#include <QList>
#include "Data/SessionInfo.h"
#include "Data/Message.h"

class QListWidget;
class QListWidgetItem;
class QLabel;

class HistoryPanel : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryPanel(QWidget* parent = nullptr);

    void loadSessions(const QList<SessionInfo>& sessions);
    void addSession(const SessionInfo& session);

signals:
    void sessionSelected(int sessionId);

private slots:
    void onItemClicked(QListWidgetItem* item);

private:
    QListWidget* m_list;
    QLabel*      m_titleLabel;

    void setupUi();
};
