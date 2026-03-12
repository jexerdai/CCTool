#include "RichTextEditor.h"
#include <QKeyEvent>

RichTextEditor::RichTextEditor(QWidget* parent) : QTextEdit(parent)
{
    setObjectName("RichTextEditor");
    setPlaceholderText("输入消息，Ctrl+Enter 发送...");
    setFixedHeight(120);
    setAcceptRichText(false);   // 纯文本模式，避免粘贴带格式内容
}

void RichTextEditor::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return &&
        event->modifiers() & Qt::ControlModifier)
    {
        QString text = toPlainText().trimmed();
        if (!text.isEmpty()) {
            emit submitted(text);
            clear();
        }
        return;
    }
    QTextEdit::keyPressEvent(event);
}
