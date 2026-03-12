#pragma once
#include <QObject>
#include <QStringList>

class QProcess;

class CCProcess : public QObject
{
    Q_OBJECT
public:
    explicit CCProcess(QObject* parent = nullptr);
    ~CCProcess();

    void send(const QString& prompt,
              const QString& repoPath,
              const QString& ccSessionId = QString());

    bool isBusy() const { return m_busy; }

signals:
    void responseReady(const QString& content,
                       const QString& newSessionId,
                       const QStringList& steps);
    void errorOccurred(const QString& error);

private slots:
    void onProcessFinished(int exitCode);
    void onProcessError();

private:
    QProcess* m_process;
    bool      m_busy = false;

    void parseOutput(const QByteArray& raw);
};
