#include "TalkView.h"
#include <QScrollBar>

TalkView::TalkView(QWidget* parent) : QTextBrowser(parent)
{
    setObjectName("TalkView");
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

void TalkView::appendUserMessage(const QString& text)
{
    m_assistantBlockOpen = false;
    QString escaped = text.toHtmlEscaped().replace("\n", "<br/>");
    appendHtml(
        "<div class='bubble-user'>"
        "<span class='user'>You</span><br/>"
        "<span>" + escaped + "</span>"
        "</div>"
    );
}

void TalkView::beginAssistantMessage()
{
    m_assistantBlockOpen = true;
    appendHtml(
        "<div class='bubble-assistant'>"
        "<span class='assistant'><b style='color:#a6e3a1;'>Claude</b><br/>"
    );
}

void TalkView::appendAssistantChunk(const QString& chunk)
{
    if (!m_assistantBlockOpen) return;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    insertHtml(chunk.toHtmlEscaped().replace("\n", "<br/>"));
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void TalkView::endAssistantMessage()
{
    if (!m_assistantBlockOpen) return;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    insertHtml("</span></div>");
    m_assistantBlockOpen = false;
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void TalkView::appendAssistantMessage(const QString& text)
{
    m_assistantBlockOpen = false;
    QString escaped = text.toHtmlEscaped().replace("\n", "<br/>");
    appendHtml(
        "<div class='bubble-assistant'>"
        "<span class='assistant'><b style='color:#a6e3a1;'>Claude</b><br/>"
        + escaped +
        "</span></div>"
    );
}

void TalkView::appendStepInfo(const QString& text)
{
    QString escaped = text.toHtmlEscaped();
    appendHtml("<div class='bubble-step'><span class='step'>● " + escaped + "</span></div>");
}

void TalkView::appendSteps(const QStringList& steps)
{
    for (const QString& s : steps)
        appendStepInfo(s);
}

void TalkView::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void TalkView::clear()
{
    QTextBrowser::clear();
    m_assistantBlockOpen = false;
}

void TalkView::appendHtml(const QString& html)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
    insertHtml(html);
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}
