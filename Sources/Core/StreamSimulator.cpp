#include "StreamSimulator.h"
#include "../Ui/OutputView.h"
#include <QTimer>

StreamSimulator::StreamSimulator(QObject* parent) : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &StreamSimulator::onTick);
}

StreamSimulator::~StreamSimulator()
{
    m_timer->stop();
}

void StreamSimulator::start(OutputView* view, const QString& fullText, int intervalMs)
{
    m_timer->stop();
    m_view  = view;
    m_text  = fullText;
    m_index = 0;

    if (fullText.isEmpty()) {
        emit finished();
        return;
    }

    // 插入"Claude:"前缀行，后续 appendAssistantChunk 直接追加
    m_view->appendHtml(
        "<div style='background:#262637;border-radius:8px;"
        "padding:10px;margin:6px 0;color:#cdd6f4;'>"
        "<b style='color:#a6e3a1;'>Claude</b><br>"
        "</div>"
    );

    m_timer->start(intervalMs);
}

void StreamSimulator::stop()
{
    m_timer->stop();
    m_view  = nullptr;
    m_index = 0;
}

void StreamSimulator::onTick()
{
    if (!m_view || m_index >= m_text.size()) {
        m_timer->stop();
        m_view  = nullptr;
        emit finished();
        return;
    }

    int end     = qMin(m_index + m_chunkSize, m_text.size());
    QString chunk = m_text.mid(m_index, end - m_index);
    m_index = end;

    m_view->appendAssistantChunk(chunk);
}
