#include "TalkView.h"
#include <QScrollBar>
#include <QTimer>
#include <QHBoxLayout>

// ── 样式常量 ─────────────────────────────────────────────────────────────────

static const QString kBgPage      = "#1a1b26";   // 页面背景
static const QString kBgUser      = "#1e3a5f";   // 用户气泡背景
static const QString kBorderUser  = "#2d5a8e";   // 用户气泡边框
static const QString kTextUser    = "#e0eaf8";   // 用户文字
static const QString kBgClaude    = "#1e2a1e";   // Claude 气泡背景
static const QString kBorderClaude= "#2a4a2a";   // Claude 气泡边框
static const QString kTextClaude  = "#c8e6c8";   // Claude 文字
static const QString kBgStep      = "#1e1e2e";   // 步骤背景
static const QString kTextStep    = "#737aa2";   // 步骤文字
static const QString kFont        = "Consolas, 'Cascadia Code', 'Courier New', monospace";

// ── 构造 ─────────────────────────────────────────────────────────────────────

TalkView::TalkView(QWidget* parent) : QScrollArea(parent)
{
    setObjectName("TalkView");
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);

    m_container = new QWidget(this);
    m_container->setObjectName("TalkViewContainer");
    m_container->setStyleSheet(QString("background:%1;").arg(kBgPage));

    m_layout = new QVBoxLayout(m_container);
    m_layout->setContentsMargins(12, 12, 12, 12);
    m_layout->setSpacing(8);
    m_layout->addStretch();   // 把气泡推到底部

    setWidget(m_container);
}

// ── 气泡工厂 ─────────────────────────────────────────────────────────────────

QLabel* TalkView::makeBubble(const QString& text, bool isUser)
{
    auto* label = new QLabel(m_container);
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);
    label->setText(text.toHtmlEscaped().replace("\n", "<br/>"));

    // 气泡最大宽度 = 容器宽度 * 0.75，最小不限
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QString bg     = isUser ? kBgUser     : kBgClaude;
    QString border = isUser ? kBorderUser : kBorderClaude;
    QString color  = isUser ? kTextUser   : kTextClaude;

    label->setStyleSheet(QString(
        "background:%1;"
        "border:1px solid %2;"
        "border-radius:12px;"
        "padding:10px 14px;"
        "color:%3;"
        "font-family:%4;"
        "font-size:13px;"
    ).arg(bg, border, color, kFont));

    return label;
}

QLabel* TalkView::makeStep(const QString& text)
{
    auto* label = new QLabel(m_container);
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    label->setText(text);
    label->setStyleSheet(QString(
        "background:%1;"
        "border-left:2px solid #565f89;"
        "border-radius:4px;"
        "padding:3px 10px;"
        "color:%2;"
        "font-family:%3;"
        "font-size:12px;"
    ).arg(kBgStep, kTextStep, kFont));
    return label;
}

// ── 公有接口 ─────────────────────────────────────────────────────────────────

void TalkView::appendUserMessage(const QString& text)
{
    m_streamLabel = nullptr;
    QLabel* bubble = makeBubble(text, true);

    auto* row = new QHBoxLayout;
    row->addWidget(bubble);

    m_layout->insertLayout(m_layout->count() - 1, row);
    scrollToBottomDeferred();
}

void TalkView::beginAssistantMessage()
{
    QLabel* bubble = makeBubble("", false);
    m_streamLabel = bubble;

    auto* row = new QHBoxLayout;
    row->addWidget(bubble);

    m_layout->insertLayout(m_layout->count() - 1, row);
    scrollToBottomDeferred();
}

void TalkView::appendAssistantChunk(const QString& chunk)
{
    if (!m_streamLabel) return;

    // 把当前 plain text 取出来追加
    QString current = m_streamLabel->text();
    // text() returns the rendered HTML — track raw text separately via property
    QString raw = m_streamLabel->property("rawText").toString();
    raw += chunk;
    m_streamLabel->setProperty("rawText", raw);
    m_streamLabel->setText(raw.toHtmlEscaped().replace("\n", "<br/>"));

    // 宽度可能变化，重新限制
    m_streamLabel->setMaximumWidth(qMax(200, m_container->width() * 3 / 4));
    scrollToBottomDeferred();
}

void TalkView::endAssistantMessage()
{
    m_streamLabel = nullptr;
}

void TalkView::appendAssistantMessage(const QString& text)
{
    m_streamLabel = nullptr;
    QLabel* bubble = makeBubble(text, false);

    auto* row = new QHBoxLayout;
    row->addWidget(bubble);

    m_layout->insertLayout(m_layout->count() - 1, row);
    scrollToBottomDeferred();
}

void TalkView::appendStepInfo(const QString& text)
{
    QLabel* step = makeStep(text);

    auto* row = new QHBoxLayout;
    row->setContentsMargins(40, 0, 40, 0);
    row->addWidget(step);

    m_layout->insertLayout(m_layout->count() - 1, row);
    scrollToBottomDeferred();
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
    m_streamLabel = nullptr;

    // 删除所有 layout 中的 item（保留末尾的 stretch）
    while (m_layout->count() > 1) {
        QLayoutItem* item = m_layout->takeAt(0);
        if (item->layout()) {
            // 清空子 layout 里的 widget
            while (item->layout()->count()) {
                QLayoutItem* child = item->layout()->takeAt(0);
                if (child->widget()) child->widget()->deleteLater();
                delete child;
            }
            delete item->layout();
        } else if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void TalkView::scrollToBottomDeferred()
{
    QTimer::singleShot(30, this, [this]() {
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    });
}
