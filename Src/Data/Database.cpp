#include "Database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

Database& Database::instance()
{
    static Database inst;
    return inst;
}

Database::Database(QObject* parent) : QObject(parent) {}

bool Database::init(const QString& dbPath)
{
    QString path = dbPath;
    if (path.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        path = dataDir + "/cctool.db";
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);
    if (!db.open()) {
        qWarning() << "Database::init failed:" << db.lastError().text();
        return false;
    }
    return createTables();
}

bool Database::createTables()
{
    QSqlQuery q;
    const QStringList ddl = {
        R"(CREATE TABLE IF NOT EXISTS repos (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            name       TEXT NOT NULL,
            path       TEXT NOT NULL UNIQUE,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        ))",
        R"(CREATE TABLE IF NOT EXISTS sessions (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            repo_id       INTEGER NOT NULL,
            cc_session_id TEXT,
            created_at    DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(repo_id) REFERENCES repos(id) ON DELETE CASCADE
        ))",
        R"(CREATE TABLE IF NOT EXISTS messages (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            session_id INTEGER NOT NULL,
            role       TEXT NOT NULL,
            content    TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(session_id) REFERENCES sessions(id) ON DELETE CASCADE
        ))"
    };

    for (const QString& sql : ddl) {
        if (!q.exec(sql)) {
            qWarning() << "createTables error:" << q.lastError().text();
            return false;
        }
    }
    return true;
}

// ── 仓库 ────────────────────────────────────────────────────────────────────

bool Database::addRepo(RepoInfo& repo)
{
    QSqlQuery q;
    q.prepare("INSERT OR IGNORE INTO repos (name, path) VALUES (?, ?)");
    q.addBindValue(repo.name);
    q.addBindValue(repo.path);
    if (!q.exec()) {
        qWarning() << "addRepo error:" << q.lastError().text();
        return false;
    }
    repo.id = q.lastInsertId().toInt();
    return true;
}

bool Database::removeRepo(int repoId)
{
    QSqlQuery q;
    q.prepare("DELETE FROM repos WHERE id = ?");
    q.addBindValue(repoId);
    return q.exec();
}

QList<RepoInfo> Database::listRepos()
{
    QList<RepoInfo> list;
    QSqlQuery q("SELECT id, name, path FROM repos ORDER BY created_at ASC");
    while (q.next()) {
        RepoInfo r;
        r.id   = q.value(0).toInt();
        r.name = q.value(1).toString();
        r.path = q.value(2).toString();
        list.append(r);
    }
    return list;
}

// ── 会话 ────────────────────────────────────────────────────────────────────

SessionInfo Database::createSession(int repoId)
{
    QSqlQuery q;
    q.prepare("INSERT INTO sessions (repo_id) VALUES (?)");
    q.addBindValue(repoId);
    SessionInfo s;
    if (q.exec()) {
        s.id        = q.lastInsertId().toInt();
        s.repoId    = repoId;
        s.createdAt = QDateTime::currentDateTime();
    }
    return s;
}

bool Database::updateSessionCCId(int sessionId, const QString& ccSessionId)
{
    QSqlQuery q;
    q.prepare("UPDATE sessions SET cc_session_id = ? WHERE id = ?");
    q.addBindValue(ccSessionId);
    q.addBindValue(sessionId);
    return q.exec();
}

SessionInfo Database::getLatestSession(int repoId)
{
    QSqlQuery q;
    q.prepare("SELECT id, cc_session_id, created_at FROM sessions "
              "WHERE repo_id = ? ORDER BY created_at DESC LIMIT 1");
    q.addBindValue(repoId);
    SessionInfo s;
    if (q.exec() && q.next()) {
        s.id          = q.value(0).toInt();
        s.repoId      = repoId;
        s.ccSessionId = q.value(1).toString();
        s.createdAt   = q.value(2).toDateTime();
    }
    return s;
}

QList<SessionInfo> Database::listSessions(int repoId)
{
    QList<SessionInfo> list;
    QSqlQuery q;
    q.prepare("SELECT id, cc_session_id, created_at FROM sessions "
              "WHERE repo_id = ? ORDER BY created_at DESC");
    q.addBindValue(repoId);
    if (q.exec()) {
        while (q.next()) {
            SessionInfo s;
            s.id          = q.value(0).toInt();
            s.repoId      = repoId;
            s.ccSessionId = q.value(1).toString();
            s.createdAt   = q.value(2).toDateTime();
            list.append(s);
        }
    }
    return list;
}

// ── 消息 ────────────────────────────────────────────────────────────────────

bool Database::saveMessage(Message& msg)
{
    QSqlQuery q;
    q.prepare("INSERT INTO messages (session_id, role, content) VALUES (?, ?, ?)");
    q.addBindValue(msg.sessionId);
    q.addBindValue(msg.role);
    q.addBindValue(msg.content);
    if (!q.exec()) return false;
    msg.id = q.lastInsertId().toInt();
    return true;
}

QList<Message> Database::loadMessages(int sessionId)
{
    QList<Message> list;
    QSqlQuery q;
    q.prepare("SELECT id, role, content, created_at FROM messages "
              "WHERE session_id = ? ORDER BY created_at ASC");
    q.addBindValue(sessionId);
    if (q.exec()) {
        while (q.next()) {
            Message m;
            m.id        = q.value(0).toInt();
            m.sessionId = sessionId;
            m.role      = q.value(1).toString();
            m.content   = q.value(2).toString();
            m.createdAt = q.value(3).toDateTime();
            list.append(m);
        }
    }
    return list;
}
