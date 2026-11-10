#ifndef CELL_FLASH_ANIMATOR_H
#define CELL_FLASH_ANIMATOR_H

#include <QObject>
#include <QPointer>
#include <QVector>
#include <QList>

class QTimer;
class QWidget;

// Decorative continuous-looping animation: runs cycles of `eventCount` random
// cell highlights spaced `intervalMs` apart, each lasting `highlightMs`, and
// repeats automatically after each cycle completes until stop() is called.
// Once stopped, it never restarts.
class CellFlashAnimator : public QObject {
    Q_OBJECT

public:
    explicit CellFlashAnimator(QObject* parent = nullptr);

    void setCells(const QVector<QWidget*>& cells);
    void start(int eventCount, int intervalMs, int highlightMs);
    void stop();

    bool isRunning() const { return running_ && !stopped_; }
    bool hasStarted() const { return started_; }
    bool isStopped() const { return stopped_; }
    int firedEvents() const { return fired_; }
    int completedCycles() const { return cycles_; }
    int activeHighlights() const { return active_.size(); }

private:
    void scheduleCycle();
    void fireEvent();
    void clearCell(QWidget* cell);
    void untrackTimer(QTimer* timer);

    QVector<QPointer<QWidget>> cells_;
    QVector<QPointer<QWidget>> active_;
    QList<QTimer*> timers_;

    int eventCount_ = 0;
    int intervalMs_ = 100;
    int highlightMs_ = 200;

    bool started_ = false;
    bool running_ = false;
    bool stopped_ = false;
    int fired_ = 0;
    int cycles_ = 0;
};

#endif // CELL_FLASH_ANIMATOR_H
