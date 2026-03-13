#pragma once
#include <QWidget>
#include "Data/RepoInfo.h"

class TalkView;
class RichTextEditor;
class CCProcess;
class CCSessionReader;

class QSplitter;
class QPushButton;
class QLabel;

class RepoTab : public QWidget
{
    Q_OBJECT
public:
    explicit RepoTab(const RepoInfo& repo, QWidget* parent = nullptr, bool autoPickSession = true);
    ~RepoTab();

    const RepoInfo& repoInfo() const { return m_repo; }

    void loadSession(const QString& sessionId);
    void showSessionPicker();   // 双击 Tab 时弹出 Session 选择框

private slots:
    void onInitFinished();
    void promptSessionPick();
    void onSendClicked();
    void onSubmitted(const QString& text);
    void onStreamChunk(const QString& text);
    void onStreamSteps(const QStringList& steps);
    void onResponseFinished(const QString& fullContent, const QString& newSessionId);
    void onCCError(const QString& error);

private:
    void setupUi();
    void setInputEnabled(bool enabled);

    RepoInfo        m_repo;
    QString         m_currentSessionId;

    TalkView*         m_outputView;
    RichTextEditor*   m_editor;
    QPushButton*      m_sendBtn;
    QLabel*           m_statusLabel;

    CCProcess*        m_ccProcess;
    CCSessionReader*  m_sessionReader;
};
