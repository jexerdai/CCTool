#pragma once
#include <QObject>

class TalkView;
class QTimer;

class StreamSimulator : public QObject
{
    Q_OBJECT
public:
    explicit StreamSimulator(QObject* parent = nullptr);
    ~StreamSimulator();

    // 开始将 fullText 逐字/逐块写入 view
    void start(TalkView* view, const QString& fullText, int intervalMs = 12);
    void stop();

signals:
    void finished();

private slots:
    void onTick();

private:
    QTimer*     m_timer;
    TalkView* m_view    = nullptr;
    QString     m_text;
    int         m_index   = 0;
    int         m_chunkSize = 3;   // 每次写入字符数（调整速度感）
};
