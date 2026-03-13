#pragma once
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>

class QScrollArea;
class QWidget;
class QVBoxLayout;

class TalkView : public QScrollArea
{
    Q_OBJECT
public:
    explicit TalkView(QWidget* parent = nullptr);

    void appendUserMessage(const QString& text);
    void beginAssistantMessage();
    void appendAssistantChunk(const QString& chunk);
    void endAssistantMessage();
    void appendAssistantMessage(const QString& text);
    void appendStepInfo(const QString& text);
    void appendSteps(const QStringList& steps);
    void scrollToBottom();
    void clear();

    // kept for compatibility (no-op for QScrollArea based impl)
    void appendHtml(const QString&) {}

private:
    QWidget*     m_container;
    QVBoxLayout* m_layout;
    QLabel*      m_streamLabel = nullptr;  // 当前流式气泡

    QLabel* makeBubble(const QString& text, bool isUser);
    QLabel* makeStep(const QString& text);
    void    scrollToBottomDeferred();
};
