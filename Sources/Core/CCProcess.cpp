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
    // 只移除 CLAUDE_CODE_* 和 CLAUDECODE* 变量，防止"nested session"检测触发
    // 不移除 CLAUDE_API_KEY 等合法变量
    const QStringList keys = env.keys();
    for (const QString& key : keys) {
        QString upper = key.toUpper();
        if (upper == "CLAUDECODE" ||
            upper.startsWith("CLAUDE_CODE_") ||
            upper == "CLAUDECODE_SESSION_ID" ||
            upper == "CLAUDE_CODE_SESSION_ID")
        {
            env.remove(key);
        }
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

    QStringList args = { "-p", prompt, "--output-format", "stream-json", "--verbose" };
    if (!ccSessionId.isEmpty())
        args << "--resume" << ccSessionId;

    qDebug() << "[CCProcess] start:" << "claude" << args;
    m_process->start("claude", args);
}

void CCProcess::onReadyRead()
{
    if (m_mode == Mode::Init) return;

    QByteArray newData = m_process->readAllStandardOutput();
    qDebug() << "[CCProcess] onReadyRead bytes:" << newData.size();
    m_buffer += newData;

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

        if (type == "assistant") {
            QJsonArray contentArr = obj.value("message").toObject()
                                       .value("content").toArray();
            QStringList steps;
            QString textChunk;

            for (const QJsonValue& v : contentArr) {
                QJsonObject block = v.toObject();
                QString blockType = block.value("type").toString();

                if (blockType == "text") {
                    textChunk += block.value("text").toString();
                } else if (blockType == "tool_use") {
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

            if (!textChunk.isEmpty()) {
                m_fullContent += textChunk;
                emit streamChunk(textChunk);
            }
            if (!steps.isEmpty())
                emit streamSteps(steps);

        } else if (type == "content_block_delta") {
            // 兼容旧版 stream-json 格式
            QString delta = obj.value("delta").toObject()
                               .value("text").toString();
            if (!delta.isEmpty()) {
                m_fullContent += delta;
                emit streamChunk(delta);
            }
        } else if (type == "result") {
            m_lastSessionId = obj.value("session_id").toString();
        } else if (type == "system") {
            // 忽略 system 消息（hook events 等）
        }
    }
}

void CCProcess::onProcessFinished(int exitCode)
{
    m_busy = false;
    Mode finishedMode = m_mode;
    m_mode = Mode::Send;

    // 进程结束时把剩余 stdout 全部读出来处理
    QByteArray remaining = m_process->readAllStandardOutput();
    if (!remaining.isEmpty()) {
        qDebug() << "[CCProcess] remaining stdout on finish:" << remaining.size();
        m_buffer += remaining;
        // 触发一次解析
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
            if (type == "assistant") {
                QJsonArray arr = obj.value("message").toObject().value("content").toArray();
                for (const QJsonValue& v : arr) {
                    QJsonObject block = v.toObject();
                    if (block.value("type").toString() == "text") {
                        QString t = block.value("text").toString();
                        if (!t.isEmpty()) { m_fullContent += t; emit streamChunk(t); }
                    }
                }
            } else if (type == "result") {
                m_lastSessionId = obj.value("session_id").toString();
            }
        }
    }

    QString errText = QString::fromUtf8(m_process->readAllStandardError());
    qDebug() << "[CCProcess] finished exitCode:" << exitCode << "mode:" << (int)finishedMode;
    if (!errText.isEmpty()) qDebug() << "[CCProcess] stderr:" << errText;

    if (exitCode != 0) {
        emit errorOccurred(errText.isEmpty() ? QString("进程退出码: %1").arg(exitCode) : errText);
        return;
    }

    if (finishedMode == Mode::Init) {
        emit initFinished();
        return;
    }

    // stderr 有内容但 exit==0 时也显示（claude 有时把错误写到 stderr 但不设非零退出码）
    if (m_fullContent.isEmpty() && !errText.isEmpty()) {
        emit errorOccurred(errText);
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
