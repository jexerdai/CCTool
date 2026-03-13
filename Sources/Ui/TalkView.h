#pragma once
#include <QTextBrowser>

class TalkView : public QTextBrowser
{
    Q_OBJECT
public:
    explicit TalkView(QWidget* parent = nullptr);

    void appendUserMessage(const QString& text);
    void beginAssistantMessage();                  // 开启流式助手气泡
    void appendAssistantChunk(const QString& chunk); // 流式追加文本块
    void endAssistantMessage();                    // 结束流式助手气泡
    void appendAssistantMessage(const QString& text); // 历史回溯用（完整消息）
    void appendStepInfo(const QString& text);
    void appendSteps(const QStringList& steps);
    void scrollToBottom();
    void clear();
    void appendHtml(const QString& html);

private:
    bool m_assistantBlockOpen = false;
};
