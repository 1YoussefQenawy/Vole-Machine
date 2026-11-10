#ifndef MACHINE_STATUS_PANEL_H
#define MACHINE_STATUS_PANEL_H

#include <QFrame>
#include <QLabel>
#include "../VoleMachine.h"

enum class ExecutionState {
    Ready,
    Running,
    Halted,
    Error
};

// Single merged CPU information sub-card embedded inside ScreenPanel beside
// the simulated display: shows execution status badge, PC, and IR together.
class MachineStatusPanel : public QFrame {
    Q_OBJECT

public:
    explicit MachineStatusPanel(QWidget* parent = nullptr);

    void updateStatus(Byte pc, Word ir, ExecutionState state);
    static QString describeInstruction(Word ir);
    static QString stateKey(ExecutionState state);

private:
    QLabel* statusBadgeLabel_ = nullptr;
    QLabel* pcHexLabel_ = nullptr;
    QLabel* pcDecLabel_ = nullptr;
    QLabel* irHexLabel_ = nullptr;
    QLabel* irDecodedLabel_ = nullptr;
};

#endif // MACHINE_STATUS_PANEL_H
