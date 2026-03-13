#include "CCProcess.h"
#include <QProcess>
#include <QProcessEnvironment>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

CCProcess::CCProcess(QObject* parent) : QObject(parent)
{
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);


    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &CCProcess::onReadyRead);
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

void CCProcess::applyCleanEnv()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // 移除所有 CLAUDE* 变量，防止"nested session"检测触发
    const QStringList keys = env.keys();
    for (const QString& key : keys) {
        if (key.startsWith(QLatin1String("CLAUDE"), Qt::CaseInsensitive))
            env.remove(key);
    }
    m_process->setProcessEnvironment(env);
}

void CCProcess::init(const QString& repoPath)
{
    if (m_busy) return;
    m_busy = true;
    m_mode = Mode::Init;

    applyCleanEnv();
    m_process->setWorkingDirectory(repoPath);
    m_process->start("claude", QStringList{ "--init" });
}

void CCProcess::send(const QString& prompt,
                     const QString& repoPath,
                     const QString& ccSessionId)
{
    if (m_busy) return;
    m_busy        = true;
    m_mode        = Mode::Send;
    m_buffer.clear();
    m_fullContent.clear();
    m_lastSessionId.clear();

    applyCleanEnv();
    m_process->setWorkingDirectory(repoPath);

    QStringList args = { "-p", prompt, "--output-format", "stream-json" };
    if (!ccSessionId.isEmpty())
        args << "--resume" << ccSessionId;

    m_process->start("claude", args);
}

void CCProcess::onReadyRead()
{
    if (m_mode == Mode::Init) return;

    m_buffer += m_process->readAllStandardOutput();

    // 按行处理，未结尾的行留在 buffer
    int newline;
    while ((newline = m_buffer.indexOf('\n')) != -1) {
        QByteArray line = m_buffer.left(newline).trimmed();
        m_buffer = m_buffer.mid(newline + 1);

        if (line.isEmpty()) continue;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) continue;

        QJsonObject obj = doc.object();
        QString type = obj.value("type").toString();

        if (type == "content_block_delta") {
            // 流式文本 delta
            QString delta = obj.value("delta").toObject()
                               .value("text").toString();
            if (!delta.isEmpty()) {
                m_fullContent += delta;
                emit streamChunk(delta);
            }
        } else if (type == "result") {
            m_lastSessionId = obj.value("session_id").toString();
        } else if (type == "assistant") {
            // 工具调用步骤
            QStringList steps;
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
            if (!steps.isEmpty())
                emit streamSteps(steps);
        }
    }
}

void CCProcess::onProcessFinished(int exitCode)
{
    m_busy = false;
    Mode finishedMode = m_mode;
    m_mode = Mode::Send;

    if (exitCode != 0) {
        QString errText = QString::fromUtf8(m_process->readAllStandardError());
        emit errorOccurred(errText.isEmpty() ? QString("进程退出码: %1").arg(exitCode) : errText);
        return;
    }

    if (finishedMode == Mode::Init) {
        emit initFinished();
        return;
    }

    emit responseFinished(m_fullContent, m_lastSessionId);
}

void CCProcess::onProcessError()
{
    m_busy = false;
    emit errorOccurred("无法启动 claude 进程：" + m_process->errorString() +
                       "\n请确认 claude CLI 已安装并在 PATH 中");
}
