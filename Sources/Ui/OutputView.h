#pragma once
#include <QTextBrowser>

class OutputView : public QTextBrowser
{
    Q_OBJECT
public:
    explicit OutputView(QWidget* parent = nullptr);

    void appendUserMessage(const QString& text);
    void appendAssistantChunk(const QString& chunk);   // 用于 StreamSimulator 逐字追加
    void appendAssistantMessage(const QString& text);  // 直接追加完整消息
    void appendStepInfo(const QString& text);          // 灰色显示工具调用步骤
    void appendSteps(const QStringList& steps);
    void clear();

    void appendHtml(const QString& html);   // StreamSimulator 也需要调用

private:
    bool m_assistantBlockOpen = false;  // 是否正在追加助手消息块
};
