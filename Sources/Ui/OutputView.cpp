#include "OutputView.h"
#include <QScrollBar>

OutputView::OutputView(QWidget* parent) : QTextBrowser(parent)
{
    setObjectName("OutputView");
    setReadOnly(true);
    setOpenLinks(false);
    document()->setDefaultStyleSheet(
        "body { font-family: 'Segoe UI', sans-serif; font-size: 13px; }"
        ".user { color: #89b4fa; font-weight: bold; }"
        ".assistant { color: #cdd6f4; }"
        ".step { color: #6c7086; font-size: 12px; font-style: italic; }"
        ".bubble-user { background:#1a2744; border-radius:8px; padding:10px; margin:6px 0; }"
        ".bubble-assistant { background:#262637; border-radius:8px; padding:10px; margin:6px 0; }"
        ".bubble-step { background:#1e1e2e; border-left:3px solid #45475a; padding:4px 8px; margin:2px 0; }"
    );
}

void OutputView::appendUserMessage(const QString& text)
{
    m_assistantBlockOpen = false;
    QString escaped = text.toHtmlEscaped().replace("\n", "<br>");
    appendHtml(
        "<div class='bubble-user'>"
        "<span class='user'>You</span><br>"
        "<span>" + escaped + "</span>"
        "</div>"
    );
}

void OutputView::appendStepInfo(const QString& text)
{
    QString escaped = text.toHtmlEscaped();
    appendHtml("<div class='bubble-step'><span class='step'>● " + escaped + "</span></div>");
}

void OutputView::appendSteps(const QStringList& steps)
{
    for (const QString& s : steps)
        appendStepInfo(s);
}

void OutputView::appendAssistantMessage(const QString& text)
{
    m_assistantBlockOpen = false;
    QString escaped = text.toHtmlEscaped().replace("\n", "<br>");
    appendHtml(
        "<div class='bubble-assistant'>"
        "<span class='assistant'>" + escaped + "</span>"
        "</div>"
    );
}

// StreamSimulator 调用此方法逐字追加
void OutputView::appendAssistantChunk(const QString& chunk)
{
    // 移动光标到末尾，直接插入纯文本以保留格式
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    insertPlainText(chunk);
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void OutputView::clear()
{
    QTextBrowser::clear();
    m_assistantBlockOpen = false;
}

void OutputView::appendHtml(const QString& html)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    insertHtml(html);
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}
