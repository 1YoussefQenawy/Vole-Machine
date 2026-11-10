#include "CellFlashAnimator.h"
#include "ThemeConfig.h"

#include <QRandomGenerator>
#include <QTimer>
#include <QVariant>
#include <QWidget>

namespace {
const char* const kFlashProperty = "flash";
}

CellFlashAnimator::CellFlashAnimator(QObject* parent)
    : QObject(parent) {}

void CellFlashAnimator::setCells(const QVector<QWidget*>& cells) {
    cells_.clear();
    cells_.reserve(cells.size());
    for (QWidget* cell : cells) {
        cells_.append(QPointer<QWidget>(cell));
    }
}

void CellFlashAnimator::start(int eventCount, int intervalMs, int highlightMs) {
    // Prevent duplicate animation loops or restarting after stop()
    if (started_ || running_ || stopped_ || cells_.isEmpty() || eventCount <= 0) {
        return;
    }
    started_ = true;
    running_ = true;
    eventCount_ = eventCount;
    intervalMs_ = intervalMs;
    highlightMs_ = highlightMs;

    scheduleCycle();
}

void CellFlashAnimator::scheduleCycle() {
    if (stopped_ || !running_) return;

    for (int i = 0; i < eventCount_; ++i) {
        auto* eventTimer = new QTimer(this);
        eventTimer->setSingleShot(true);
        eventTimer->setInterval(i * intervalMs_);
        connect(eventTimer, &QTimer::timeout, this, [this, eventTimer]() {
            untrackTimer(eventTimer);
            fireEvent();
        });
        timers_.append(eventTimer);
        eventTimer->start();
    }

    // Schedule next cycle immediately after all events in this cycle finish their highlights
    const int cycleDurationMs = (eventCount_ - 1) * intervalMs_ + highlightMs_;
    auto* nextCycleTimer = new QTimer(this);
    nextCycleTimer->setSingleShot(true);
    nextCycleTimer->setInterval(cycleDurationMs);
    connect(nextCycleTimer, &QTimer::timeout, this, [this, nextCycleTimer]() {
        untrackTimer(nextCycleTimer);
        if (stopped_ || !running_) return;
        ++cycles_;
        scheduleCycle();
    });
    timers_.append(nextCycleTimer);
    nextCycleTimer->start();
}

void CellFlashAnimator::stop() {
    stopped_ = true;
    running_ = false;

    const auto pending = timers_;
    timers_.clear();
    for (QTimer* timer : pending) {
        if (timer) {
            timer->stop();
            timer->deleteLater();
        }
    }

    const auto stillActive = active_;
    active_.clear();
    for (const QPointer<QWidget>& cell : stillActive) {
        if (cell) {
            cell->setProperty(kFlashProperty, QVariant());
            ThemeConfig::refreshWidgetStyle(cell);
        }
    }
}

void CellFlashAnimator::fireEvent() {
    if (stopped_ || !running_) return;

    QVector<int> candidates;
    candidates.reserve(cells_.size());
    for (int i = 0; i < cells_.size(); ++i) {
        if (cells_[i] && !active_.contains(cells_[i])) {
            candidates.append(i);
        }
    }
    if (candidates.isEmpty()) return;

    auto* rng = QRandomGenerator::global();
    QPointer<QWidget> cell = cells_[candidates[rng->bounded(static_cast<int>(candidates.size()))]];
    const QStringList& colors = ThemeConfig::FLASH_COLOR_KEYS;
    const QString color = colors.at(rng->bounded(static_cast<int>(colors.size())));

    cell->setProperty(kFlashProperty, color);
    ThemeConfig::refreshWidgetStyle(cell);
    active_.append(cell);
    ++fired_;

    auto* restoreTimer = new QTimer(this);
    restoreTimer->setSingleShot(true);
    restoreTimer->setInterval(highlightMs_);
    connect(restoreTimer, &QTimer::timeout, this, [this, restoreTimer, cell]() {
        untrackTimer(restoreTimer);
        clearCell(cell);
    });
    timers_.append(restoreTimer);
    restoreTimer->start();
}

void CellFlashAnimator::clearCell(QWidget* cell) {
    if (!cell) return;
    cell->setProperty(kFlashProperty, QVariant());
    ThemeConfig::refreshWidgetStyle(cell);
    active_.removeAll(QPointer<QWidget>(cell));
}

void CellFlashAnimator::untrackTimer(QTimer* timer) {
    if (!timer) return;
    timers_.removeAll(timer);
    timer->deleteLater();
}
