#include "RepoTab.h"
#include "OutputView.h"
#include "RichTextEditor.h"
#include "HistoryPanel.h"
#include "Core/CCProcess.h"
#include "Core/StreamSimulator.h"
#include "Data/Database.h"

#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

RepoTab::RepoTab(const RepoInfo& repo, QWidget* parent)
    : QWidget(parent)
    , m_repo(repo)
    , m_ccProcess(new CCProcess(this))
    , m_streamSim(new StreamSimulator(this))
{
    setupUi();

    // 加载该仓库的历史会话列表
    auto sessions = Database::instance().listSessions(m_repo.id);
    m_historyPanel->loadSessions(sessions);

    // 取最近会话继续（若有）
    m_currentSession = Database::instance().getLatestSession(m_repo.id);
    if (!m_currentSession.isValid()) {
        m_currentSession = Database::instance().createSession(m_repo.id);
    }

    connect(m_ccProcess, &CCProcess::responseReady,  this, &RepoTab::onResponseReady);
    connect(m_ccProcess, &CCProcess::errorOccurred,  this, &RepoTab::onCCError);
    connect(m_streamSim, &StreamSimulator::finished, this, &RepoTab::onStreamFinished);
    connect(m_editor,    &RichTextEditor::submitted, this, &RepoTab::onSubmitted);
    connect(m_sendBtn,   &QPushButton::clicked,      this, &RepoTab::onSendClicked);
    connect(m_historyPanel, &HistoryPanel::sessionSelected, this, &RepoTab::onSessionSelected);
}

RepoTab::~RepoTab() {}

void RepoTab::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 上方：OutputView + HistoryPanel 横向分割
    auto* hSplitter = new QSplitter(Qt::Horizontal, this);

    m_outputView  = new OutputView(this);
    m_historyPanel = new HistoryPanel(this);

    hSplitter->addWidget(m_outputView);
    hSplitter->addWidget(m_historyPanel);
    hSplitter->setStretchFactor(0, 1);
    hSplitter->setStretchFactor(1, 0);

    mainLayout->addWidget(hSplitter, 1);

    // 下方：输入区
    auto* inputWidget = new QWidget(this);
    inputWidget->setObjectName("InputPanel");
    auto* inputLayout = new QVBoxLayout(inputWidget);
    inputLayout->setContentsMargins(8, 6, 8, 8);
    inputLayout->setSpacing(4);

    m_editor = new RichTextEditor(inputWidget);
    inputLayout->addWidget(m_editor);

    auto* bottomRow = new QHBoxLayout;
    m_statusLabel = new QLabel(inputWidget);
    m_statusLabel->setStyleSheet("color:#6c7086; font-size:11px;");
    m_sendBtn = new QPushButton("发送", inputWidget);
    m_sendBtn->setObjectName("SendButton");
    m_sendBtn->setFixedSize(80, 32);

    bottomRow->addWidget(m_statusLabel, 1);
    bottomRow->addWidget(m_sendBtn);
    inputLayout->addLayout(bottomRow);

    mainLayout->addWidget(inputWidget);
}

void RepoTab::setInputEnabled(bool enabled)
{
    m_editor->setEnabled(enabled);
    m_sendBtn->setEnabled(enabled);
    m_statusLabel->setText(enabled ? "" : "正在等待 Claude Code 响应...");
}

void RepoTab::onSendClicked()
{
    onSubmitted(m_editor->toPlainText().trimmed());
    m_editor->clear();
}

void RepoTab::onSubmitted(const QString& text)
{
    if (text.isEmpty()) return;

    // 保存用户消息
    Message userMsg;
    userMsg.sessionId = m_currentSession.id;
    userMsg.role      = "user";
    userMsg.content   = text;
    Database::instance().saveMessage(userMsg);

    m_outputView->appendUserMessage(text);
    setInputEnabled(false);

    m_ccProcess->send(text, m_repo.path, m_currentSession.ccSessionId);
}

void RepoTab::onResponseReady(const QString& content,
                               const QString& newCCSessionId,
                               const QStringList& steps)
{
    // 更新 session id
    if (!newCCSessionId.isEmpty() && newCCSessionId != m_currentSession.ccSessionId) {
        m_currentSession.ccSessionId = newCCSessionId;
        Database::instance().updateSessionCCId(m_currentSession.id, newCCSessionId);
    }

    // 保存助手消息
    Message assistantMsg;
    assistantMsg.sessionId = m_currentSession.id;
    assistantMsg.role      = "assistant";
    assistantMsg.content   = content;
    Database::instance().saveMessage(assistantMsg);

    // 显示工具调用步骤
    m_outputView->appendSteps(steps);

    // 开始模拟流式显示
    m_streamSim->start(m_outputView, content);
}

void RepoTab::onCCError(const QString& error)
{
    m_outputView->appendStepInfo("错误: " + error);
    setInputEnabled(true);
}

void RepoTab::onStreamFinished()
{
    setInputEnabled(true);
}

void RepoTab::onSessionSelected(int sessionId)
{
    loadSession(sessionId);
}

void RepoTab::loadSession(int sessionId)
{
    m_outputView->clear();
    auto messages = Database::instance().loadMessages(sessionId);
    for (const auto& msg : messages) {
        if (msg.role == "user")
            m_outputView->appendUserMessage(msg.content);
        else
            m_outputView->appendAssistantMessage(msg.content);
    }
}
