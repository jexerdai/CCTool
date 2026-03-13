#pragma once
#include <QObject>
#include <QList>
#include "RepoInfo.h"
#include "SessionInfo.h"
#include "Message.h"

class QSqlDatabase;

class Database : public QObject
{
    Q_OBJECT
public:
    static Database& instance();

    bool init(const QString& dbPath = QString());

    // 仓库
    bool          addRepo(RepoInfo& repo);
    bool          removeRepo(int repoId);
    QList<RepoInfo> listRepos();
    bool          saveLastSessionId(int repoId, const QString& sessionId);
    bool          setRepoOpen(int repoId, bool open);
    bool          setRepoLastActive(int repoId);
    int           lastActiveRepoId();

    // 会话
    SessionInfo   createSession(int repoId);
    bool          updateSessionCCId(int sessionId, const QString& ccSessionId);
    SessionInfo   getLatestSession(int repoId);
    QList<SessionInfo> listSessions(int repoId);

    // 消息
    bool          saveMessage(Message& msg);
    QList<Message> loadMessages(int sessionId);

private:
    explicit Database(QObject* parent = nullptr);
    bool createTables();
};
