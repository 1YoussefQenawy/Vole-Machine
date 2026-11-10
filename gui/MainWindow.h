#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include "../VoleMachine.h"
#include "ProgramControlsPanel.h"
#include "MachineStatusPanel.h"
#include "ScreenPanel.h"
#include "RegisterGridPanel.h"
#include "MemoryMatrixPanel.h"
#include "ValueInspectorPanel.h"

class QStackedWidget;
class QTimer;
class HomePage;
class CellFlashAnimator;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    // Accessors (also used for verification)
    VoleMachine& machine() { return machine_; }
    ExecutionState executionState() const { return executionState_; }
    QString statusDetail() const { return statusDetail_; }
    bool isRunningAsync() const { return isRunningAsync_; }
    bool isWorkspaceVisible() const;
    long runStepCount() const { return runSteps_; }

    HomePage* homePage() const { return homePage_; }
    QWidget* workspace() const { return workspace_; }
    CellFlashAnimator* memoryAnimator() const { return memoryAnimator_; }
    ProgramControlsPanel* controlsPanel() const { return controlsPanel_; }
    MachineStatusPanel* statusPanel() const { return statusPanel_; }
    ScreenPanel* screenPanel() const { return screenPanel_; }
    RegisterGridPanel* registerPanel() const { return registerPanel_; }
    MemoryMatrixPanel* memoryPanel() const { return memoryPanel_; }
    ValueInspectorPanel* inspectorPanel() const { return inspectorPanel_; }

    void syncFromSimulator();

public slots:
    void showWorkspace();
    void showHomePage();
    void handleLoadProgram();
    void handleStep();
    void handleRunAll();
    void handleHalt();
    void handleReset();
    void handleRegisterWrite(int regIndex, Byte value);
    void handleMemoryWrite(int address, Byte value);
    void handleInvalidInputError(const QString& message);

signals:
    void runAllFinished();

private slots:
    void onRunTick();
    void onRegisterSelected(int regIndex);
    void onMemoryCellSelected(int address);

private:
    VoleMachine machine_;
    ExecutionState executionState_ = ExecutionState::Ready;
    QString statusDetail_;
    bool isRunningAsync_ = false;
    bool userHalted_ = false;
    long runSteps_ = 0;
    int lastPcBeforeStep_ = 0;

    QStackedWidget* stack_ = nullptr;
    HomePage* homePage_ = nullptr;
    QWidget* workspace_ = nullptr;

    ProgramControlsPanel* controlsPanel_ = nullptr;
    MachineStatusPanel* statusPanel_ = nullptr;
    ScreenPanel* screenPanel_ = nullptr;
    RegisterGridPanel* registerPanel_ = nullptr;
    MemoryMatrixPanel* memoryPanel_ = nullptr;
    ValueInspectorPanel* inspectorPanel_ = nullptr;

    CellFlashAnimator* memoryAnimator_ = nullptr;
    QTimer* runTimer_ = nullptr;
    QElapsedTimer runRefreshClock_;

    QWidget* buildWorkspace();
    void finishRunAll(bool haltedByUser);
    void reportStatus(ExecutionState state, const QString& message);
    void logInfo(const QString& level, const QString& message);
    void stopIntroAnimation();
    bool parseStartAddress(const QString& rawText, int& outAddress, QString& outError);
};

#endif // MAIN_WINDOW_H
