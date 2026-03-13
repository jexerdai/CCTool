#pragma once
#include <QObject>
#include <QStringList>
#include <QByteArray>

class QProcess;

class CCProcess : public QObject
{
    Q_OBJECT
public:
    explicit CCProcess(QObject* parent = nullptr);
    ~CCProcess();

    void init(const QString& repoPath);

    void send(const QString& prompt,
              const QString& repoPath,
              const QString& ccSessionId = QString());

    bool isBusy() const { return m_busy; }

signals:
    void initFinished();
    void streamChunk(const QString& text);        // 实时流式文本块
    void streamSteps(const QStringList& steps);   // 工具调用步骤
    void responseFinished(const QString& fullContent, const QString& newSessionId);
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();
    void onProcessFinished(int exitCode);
    void onProcessError();

private:
    enum class Mode { Init, Send };

    void applyCleanEnv();   // 启动前清除 CLAUDE* 环境变量

    QProcess*  m_process;
    bool       m_busy    = false;
    Mode       m_mode    = Mode::Send;
    QByteArray m_buffer;          // 未处理的数据缓冲
    QString    m_fullContent;     // 累积完整响应
    QString    m_lastSessionId;
};
