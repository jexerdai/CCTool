#pragma once
#include <QWidget>
#include "Data/RepoInfo.h"
#include "Data/SessionInfo.h"

class OutputView;
class RichTextEditor;
class HistoryPanel;
class CCProcess;
class StreamSimulator;

class QSplitter;
class QPushButton;
class QLabel;

class RepoTab : public QWidget
{
    Q_OBJECT
public:
    explicit RepoTab(const RepoInfo& repo, QWidget* parent = nullptr);
    ~RepoTab();

    const RepoInfo& repoInfo() const { return m_repo; }

    // 加载历史会话消息到 OutputView（只读回溯）
    void loadSession(int sessionId);

private slots:
    void onSendClicked();
    void onSubmitted(const QString& text);
    void onResponseReady(const QString& content, const QString& newCCSessionId, const QStringList& steps);
    void onCCError(const QString& error);
    void onStreamFinished();
    void onSessionSelected(int sessionId);

private:
    void setupUi();
    void setInputEnabled(bool enabled);

    RepoInfo        m_repo;
    SessionInfo     m_currentSession;

    OutputView*     m_outputView;
    RichTextEditor* m_editor;
    HistoryPanel*   m_historyPanel;
    QPushButton*    m_sendBtn;
    QLabel*         m_statusLabel;

    CCProcess*      m_ccProcess;
    StreamSimulator* m_streamSim;
};
