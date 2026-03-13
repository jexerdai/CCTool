#pragma once
#include <QWidget>
#include <QList>
#include "../Core/CCSessionReader.h"

class QListWidget;
class QListWidgetItem;
class QLabel;

class HistoryPanel : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryPanel(QWidget* parent = nullptr);

    void loadSessions(const QList<CCSession>& sessions);

signals:
    void sessionSelected(const QString& sessionId);

private slots:
    void onItemClicked(QListWidgetItem* item);

private:
    QListWidget* m_list;
    QLabel*      m_titleLabel;

    void setupUi();
};
