#include "StreamSimulator.h"
#include "../Ui/TalkView.h"
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

void StreamSimulator::start(TalkView* view, const QString& fullText, int intervalMs)
{
    m_timer->stop();
    m_view  = view;
    m_text  = fullText;
    m_index = 0;

    if (fullText.isEmpty()) {
        emit finished();
        return;
    }

    m_view->beginAssistantMessage();
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
