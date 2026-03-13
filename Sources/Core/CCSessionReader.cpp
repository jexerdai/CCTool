#include "CCSessionReader.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <algorithm>

CCSessionReader::CCSessionReader(QObject* parent) : QObject(parent) {}

QString CCSessionReader::projectsBasePath()
{
    return QDir::homePath() + "/.claude/projects";
}

QString CCSessionReader::repoPathToProjectDir(const QString& repoPath)
{
    // D:\SourceCodes\CCTool -> D--SourceCodes-CCTool
    // 规则：: 替换为 -，所有 \ / 替换为 -，空格替换为 -
    QString normalized = QDir::toNativeSeparators(repoPath);
    normalized.replace(':', '-');
    normalized.replace('\\', '-');
    normalized.replace('/', '-');
    normalized.replace(' ', '-');
    return normalized;
}

QList<CCSession> CCSessionReader::listSessions(const QString& repoPath)
{
    QList<CCSession> result;

    QString projectDir = projectsBasePath() + "/" + repoPathToProjectDir(repoPath);
    QDir dir(projectDir);
    if (!dir.exists()) return result;

    QFileInfoList files = dir.entryInfoList({"*.jsonl"}, QDir::Files, QDir::Time);

    for (const QFileInfo& fi : files) {
        CCSession session;
        session.sessionId = fi.completeBaseName();
        session.createdAt = fi.lastModified();

        // 读取第一条有效 user 消息作为预览
        QFile f(fi.absoluteFilePath());
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        bool found = false;
        while (!f.atEnd() && !found) {
            QByteArray line = f.readLine().trimmed();
            if (line.isEmpty()) continue;

            QJsonDocument doc = QJsonDocument::fromJson(line);
            if (!doc.isObject()) continue;

            QJsonObject obj = doc.object();
            if (obj.value("type").toString() != "user") continue;
            if (obj.value("isMeta").toBool()) continue;

            // content 可能是 string 或 array
            QString content;
            QJsonValue contentVal = obj.value("message").toObject().value("content");
            if (contentVal.isString()) {
                content = contentVal.toString();
            } else if (contentVal.isArray()) {
                for (const QJsonValue& v : contentVal.toArray()) {
                    QJsonObject block = v.toObject();
                    if (block.value("type").toString() == "text")
                        content += block.value("text").toString();
                }
            }
            if (content.startsWith("<command")) continue;
            if (content.trimmed().isEmpty()) continue;

            session.firstMessage = content.left(60).replace('\n', ' ');
            session.createdAt    = QDateTime::fromString(
                obj.value("timestamp").toString(), Qt::ISODateWithMs);
            found = true;
        }
        f.close();

        // 没有任何用户消息的 session 跳过
        if (!found) continue;

        result.append(session);
    }

    // 按时间降序（最新的在最上面）
    std::sort(result.begin(), result.end(), [](const CCSession& a, const CCSession& b) {
        return a.createdAt > b.createdAt;
    });

    return result;
}

QList<CCSessionReader::Message> CCSessionReader::loadMessages(
    const QString& repoPath, const QString& sessionId)
{
    QList<Message> result;

    QString filePath = projectsBasePath() + "/"
                     + repoPathToProjectDir(repoPath) + "/"
                     + sessionId + ".jsonl";

    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return result;

    QTextStream stream(&f);
    stream.setEncoding(QStringConverter::Utf8);

    while (!stream.atEnd()) {
        QString lineStr = stream.readLine();
        if (lineStr.isEmpty()) continue;

        QJsonDocument doc = QJsonDocument::fromJson(lineStr.toUtf8());
        if (!doc.isObject()) continue;

        QJsonObject obj = doc.object();
        QString type = obj.value("type").toString();

        if (type == "user") {
            // 跳过内部系统消息
            if (obj.value("isMeta").toBool()) continue;

            // content 可能是 string 或 array
            QString content;
            QJsonValue contentVal = obj.value("message").toObject().value("content");
            if (contentVal.isString()) {
                content = contentVal.toString();
            } else if (contentVal.isArray()) {
                for (const QJsonValue& v : contentVal.toArray()) {
                    QJsonObject block = v.toObject();
                    if (block.value("type").toString() == "text")
                        content += block.value("text").toString();
                }
            }
            if (content.startsWith("<command")) continue;
            if (content.trimmed().isEmpty()) continue;

            Message msg;
            msg.role      = "user";
            msg.content   = content;
            msg.timestamp = QDateTime::fromString(
                obj.value("timestamp").toString(), Qt::ISODateWithMs);
            result.append(msg);

        } else if (type == "assistant") {
            // 提取 text 类型的 content block
            QJsonArray contentArr = obj.value("message").toObject()
                                       .value("content").toArray();
            QString text;
            for (const QJsonValue& v : contentArr) {
                QJsonObject block = v.toObject();
                if (block.value("type").toString() == "text")
                    text += block.value("text").toString();
            }
            if (text.trimmed().isEmpty()) continue;

            Message msg;
            msg.role      = "assistant";
            msg.content   = text;
            msg.timestamp = QDateTime::fromString(
                obj.value("timestamp").toString(), Qt::ISODateWithMs);
            result.append(msg);
        }
    }
    f.close();

    return result;
}
