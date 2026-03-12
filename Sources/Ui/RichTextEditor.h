#pragma once
#include <QTextEdit>

class RichTextEditor : public QTextEdit
{
    Q_OBJECT
public:
    explicit RichTextEditor(QWidget* parent = nullptr);

signals:
    void submitted(const QString& text);   // Ctrl+Enter 触发

protected:
    void keyPressEvent(QKeyEvent* event) override;
};
