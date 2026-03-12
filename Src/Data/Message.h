#pragma once
#include <QString>
#include <QDateTime>

struct Message {
    int       id        = -1;
    int       sessionId = -1;
    QString   role;       // "user" 或 "assistant"
    QString   content;
    QDateTime createdAt;
};
