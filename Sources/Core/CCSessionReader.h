#pragma once
#include <QObject>
#include <QList>
#include <QString>
#include <QDateTime>

struct CCSession {
    QString  sessionId;
    QString  firstMessage;   // 第一条用户消息预览（前50字）
    QDateTime createdAt;
};

class CCSessionReader : public QObject
{
    Q_OBJECT
public:
    explicit CCSessionReader(QObject* parent = nullptr);

    // 根据仓库路径返回该仓库的所有 session，按时间降序
    QList<CCSession> listSessions(const QString& repoPath);

    // 读取某个 session 的所有消息（role + content）
    struct Message {
        QString role;     // "user" 或 "assistant"
        QString content;
        QDateTime timestamp;
    };
    QList<Message> loadMessages(const QString& repoPath, const QString& sessionId);

private:
    // 将仓库路径转换为 ~/.claude/projects/ 下的目录名
    // 例：D:\SourceCodes\CCTool -> D--SourceCodes-CCTool
    QString repoPathToProjectDir(const QString& repoPath);

    QString projectsBasePath();
};
