#include "CCProcess.h"
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

CCProcess::CCProcess(QObject* parent) : QObject(parent)
{
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus) { onProcessFinished(code); });
    connect(m_process, &QProcess::errorOccurred,
            this, &CCProcess::onProcessError);
}

CCProcess::~CCProcess()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(2000);
    }
}

void CCProcess::send(const QString& prompt,
                     const QString& repoPath,
                     const QString& ccSessionId)
{
    if (m_busy) return;
    m_busy = true;

    m_process->setWorkingDirectory(repoPath);

    QStringList args = { "-p", prompt, "--output-format", "json" };
    if (!ccSessionId.isEmpty())
        args << "--resume" << ccSessionId;

    m_process->start("claude", args);
}

void CCProcess::onProcessFinished(int exitCode)
{
    m_busy = false;

    if (exitCode != 0) {
        QString errText = QString::fromUtf8(m_process->readAllStandardError());
        emit errorOccurred(errText.isEmpty() ? QString("进程退出码: %1").arg(exitCode) : errText);
        return;
    }

    QByteArray raw = m_process->readAllStandardOutput();
    parseOutput(raw);
}

void CCProcess::onProcessError()
{
    m_busy = false;
    emit errorOccurred("无法启动 claude 进程：" + m_process->errorString() +
                       "\n请确认 claude CLI 已安装并在 PATH 中");
}

void CCProcess::parseOutput(const QByteArray& raw)
{
    // Claude Code --output-format json 可能输出多行 JSON（每行一个事件）
    // 最终结果在 type=="result" 的条目中
    QString content;
    QString sessionId;
    QStringList steps;

    const QList<QByteArray> lines = raw.split('\n');
    for (const QByteArray& line : lines) {
        if (line.trimmed().isEmpty()) continue;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line.trimmed(), &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) continue;

        QJsonObject obj = doc.object();
        QString type = obj.value("type").toString();

        if (type == "result") {
            content   = obj.value("result").toString();
            sessionId = obj.value("session_id").toString();
        } else if (type == "assistant") {
            // 工具调用步骤
            QJsonArray contentArr = obj.value("message").toObject()
                                       .value("content").toArray();
            for (const QJsonValue& v : contentArr) {
                QJsonObject block = v.toObject();
                if (block.value("type").toString() == "tool_use") {
                    QString toolName = block.value("name").toString();
                    QJsonObject input = block.value("input").toObject();
                    QString desc = toolName;
                    if (input.contains("path"))
                        desc += ": " + input.value("path").toString();
                    else if (input.contains("command"))
                        desc += ": " + input.value("command").toString();
                    steps.append(desc);
                }
            }
        }
    }

    if (content.isEmpty() && !raw.isEmpty()) {
        // 兜底：若解析失败，原样返回
        content = QString::fromUtf8(raw).trimmed();
    }

    emit responseReady(content, sessionId, steps);
}
