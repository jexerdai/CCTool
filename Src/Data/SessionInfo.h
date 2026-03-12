#pragma once
#include <QString>
#include <QDateTime>

struct SessionInfo {
    int       id          = -1;
    int       repoId      = -1;
    QString   ccSessionId;        // Claude Code 返回的 session id，用于 --resume
    QDateTime createdAt;

    bool isValid() const { return id >= 0; }
    bool hasSession() const { return !ccSessionId.isEmpty(); }
};
