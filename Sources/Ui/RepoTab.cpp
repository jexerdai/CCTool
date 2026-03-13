#include "RepoTab.h"
#include "OutputView.h"
#include "RichTextEditor.h"
#include "SessionPickerDialog.h"
#include "../Core/CCProcess.h"
#include "../Core/CCSessionReader.h"
#include "../Data/Database.h"

#include <QDir>
#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QScrollBar>

RepoTab::RepoTab(const RepoInfo& repo, QWidget* parent, bool autoPickSession)
    : QWidget(parent)
    , m_repo(repo)
    , m_ccProcess(new CCProcess(this))
    , m_sessionReader(new CCSessionReader(this))
{
    setupUi();

    connect(m_ccProcess, &CCProcess::initFinished,     this, &RepoTab::onInitFinished);
    connect(m_ccProcess, &CCProcess::streamChunk,      this, &RepoTab::onStreamChunk);
    connect(m_ccProcess, &CCProcess::streamSteps,      this, &RepoTab::onStreamSteps);
    connect(m_ccProcess, &CCProcess::responseFinished, this, &RepoTab::onResponseFinished);
    connect(m_ccProcess, &CCProcess::errorOccurred,    this, &RepoTab::onCCError);
    connect(m_editor,    &RichTextEditor::submitted,   this, &RepoTab::onSubmitted);
    connect(m_sendBtn,   &QPushButton::clicked,        this, &RepoTab::onSendClicked);

    // 判断是否为新仓库
    auto sessions = m_sessionReader->listSessions(m_repo.path);
    bool hasHistory = !sessions.isEmpty();

    if (!hasHistory) {
        setInputEnabled(false);
        m_statusLabel->setText("正在初始化 Claude 仓库...");
        m_ccProcess->init(m_repo.path);
    } else if (autoPickSession) {
        // 首次打开：延迟弹出 session 选择
        QTimer::singleShot(0, this, &RepoTab::promptSessionPick);
    } else {
        // 恢复模式：静默加载上次 session
        if (!m_repo.lastSessionId.isEmpty())
            loadSession(m_repo.lastSessionId);
        else
            loadSession(sessions.first().sessionId);
    }
}

RepoTab::~RepoTab() {}

void RepoTab::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_outputView = new OutputView(this);
    mainLayout->addWidget(m_outputView, 1);

    auto* inputWidget = new QWidget(this);
    inputWidget->setObjectName("InputPanel");
    auto* inputLayout = new QVBoxLayout(inputWidget);
    inputLayout->setContentsMargins(8, 6, 8, 8);
    inputLayout->setSpacing(4);

    m_editor = new RichTextEditor(inputWidget);
    inputLayout->addWidget(m_editor);

    auto* bottomRow = new QHBoxLayout;
    m_statusLabel   = new QLabel(inputWidget);
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
    if (enabled) m_statusLabel->setText("");
}

void RepoTab::showSessionPicker()
{
    // 双击时强制弹窗，不管 session 数量
    auto sessions = m_sessionReader->listSessions(m_repo.path);
    if (sessions.isEmpty()) {
        m_outputView->appendStepInfo("该仓库还没有任何对话 Session，请在下方输入框发送第一条消息开始对话。");
        return;
    }

    SessionPickerDialog dlg(sessions, m_currentSessionId, this);
    if (dlg.exec() == QDialog::Accepted) {
        QString picked = dlg.selectedSessionId();
        if (!picked.isEmpty())
            loadSession(picked);
    }
}

void RepoTab::promptSessionPick()
{
    auto sessions = m_sessionReader->listSessions(m_repo.path);
    if (sessions.isEmpty()) return;

    if (sessions.size() == 1) {
        // 只有一个 session，直接加载，不打扰用户
        loadSession(sessions.first().sessionId);
        return;
    }

    // 多个 session 时弹出选择框
    SessionPickerDialog dlg(sessions, m_repo.lastSessionId, this);
    if (dlg.exec() == QDialog::Accepted) {
        QString picked = dlg.selectedSessionId();
        if (!picked.isEmpty())
            loadSession(picked);
    }
    // 取消时 Tab 保持空白，等待用户输入新对话
}

void RepoTab::onInitFinished()
{
    m_outputView->appendStepInfo("Claude 仓库初始化完成");
    setInputEnabled(true);
}

void RepoTab::onSendClicked()
{
    onSubmitted(m_editor->toPlainText().trimmed());
    m_editor->clear();
}

void RepoTab::onSubmitted(const QString& text)
{
    if (text.isEmpty()) return;

    m_outputView->appendUserMessage(text);
    m_outputView->beginAssistantMessage();

    setInputEnabled(false);
    m_statusLabel->setText("正在等待 Claude Code 响应...");

    m_ccProcess->send(text, m_repo.path, m_currentSessionId);
}

void RepoTab::onStreamChunk(const QString& text)
{
    m_outputView->appendAssistantChunk(text);
}

void RepoTab::onStreamSteps(const QStringList& steps)
{
    m_outputView->appendSteps(steps);
}

void RepoTab::onResponseFinished(const QString& fullContent, const QString& newSessionId)
{
    if (!newSessionId.isEmpty()) {
        m_currentSessionId = newSessionId;
        if (m_repo.id >= 0)
            Database::instance().saveLastSessionId(m_repo.id, newSessionId);
    }

    m_outputView->endAssistantMessage();
    setInputEnabled(true);
}

void RepoTab::onCCError(const QString& error)
{
    m_outputView->appendStepInfo("错误: " + error);
    setInputEnabled(true);
}

void RepoTab::loadSession(const QString& sessionId)
{
    m_currentSessionId = sessionId;

    // 每次切换 session 都持久化，不依赖发消息触发
    if (m_repo.id >= 0)
        Database::instance().saveLastSessionId(m_repo.id, sessionId);

    m_outputView->clear();

    auto messages = m_sessionReader->loadMessages(m_repo.path, sessionId);
    if (messages.isEmpty()) {
        m_outputView->appendStepInfo("Session: " + sessionId + "（无消息记录）");
        return;
    }

    // 只渲染最近 50 条，避免大 session 阻塞 UI
    const int maxDisplay = 50;
    int start = qMax(0, messages.size() - maxDisplay);
    if (start > 0)
        m_outputView->appendStepInfo(QString("（仅显示最近 %1 条，共 %2 条）")
                                     .arg(maxDisplay).arg(messages.size()));

    for (int i = start; i < messages.size(); ++i) {
        const auto& msg = messages[i];
        if (msg.role == "user")
            m_outputView->appendUserMessage(msg.content);
        else
            m_outputView->appendAssistantMessage(msg.content);
    }

    // 延迟滚动到底部，确保渲染完成后再执行
    QTimer::singleShot(50, m_outputView, [this]() {
        m_outputView->verticalScrollBar()->setValue(m_outputView->verticalScrollBar()->maximum());
    });
}
