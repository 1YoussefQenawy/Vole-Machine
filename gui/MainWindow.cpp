#include "MainWindow.h"
#include "CellFlashAnimator.h"
#include "HomePage.h"
#include "ThemeConfig.h"

#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

namespace {

// Same infinite-loop protection as VoleMachine::runAll()
constexpr long RUN_MAX_STEPS = 1000000;
// Instructions executed per event-loop slice during Run All. Between slices the
// GUI stays responsive and a Halt request can reach the CPU safely (same thread).
constexpr int RUN_BATCH_STEPS = 500;
constexpr int RUN_UI_REFRESH_MS = 50;

QString hex2(int v) { return "0x" + QString("%1").arg(v, 2, 16, QChar('0')).toUpper(); }
QString hex4(int v) { return "0x" + QString("%1").arg(v, 4, 16, QChar('0')).toUpper(); }

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setObjectName("VoleMainWindow");
    setWindowTitle("Vole Machine Simulator");
    resize(ThemeConfig::WINDOW_DEFAULT_WIDTH, ThemeConfig::WINDOW_DEFAULT_HEIGHT);
    setMinimumSize(ThemeConfig::WINDOW_MIN_WIDTH, ThemeConfig::WINDOW_MIN_HEIGHT);
    setMenuBar(nullptr);

    stack_ = new QStackedWidget(this);
    stack_->setObjectName("PageStack");
    setCentralWidget(stack_);

    homePage_ = new HomePage(stack_);
    workspace_ = buildWorkspace();
    stack_->addWidget(homePage_);
    stack_->addWidget(workspace_);
    stack_->setCurrentWidget(homePage_);

    connect(homePage_, &HomePage::startRequested, this, &MainWindow::showWorkspace);

    // Run All driver (GUI thread, time-sliced)
    runTimer_ = new QTimer(this);
    runTimer_->setInterval(0);
    connect(runTimer_, &QTimer::timeout, this, &MainWindow::onRunTick);

    // Decorative memory intro animation (runs once when the workspace first opens)
    memoryAnimator_ = new CellFlashAnimator(this);
    memoryAnimator_->setCells(memoryPanel_->cellWidgets());

    QTimer::singleShot(ThemeConfig::HOME_INTRO_DELAY_MS, homePage_, &HomePage::startIntroAnimation);

    memoryPanel_->setSelectedAddress(0);
    inspectorPanel_->inspectMemoryCell(0, machine_.getMemory().read(0));
    reportStatus(ExecutionState::Ready, "Machine initialized. Load a program to begin.");
    syncFromSimulator();
}

QWidget* MainWindow::buildWorkspace() {
    auto* workspace = new QWidget(stack_);
    workspace->setObjectName("CentralWorkspace");
    workspace->setAttribute(Qt::WA_StyledBackground, true);

    auto* rootLayout = new QHBoxLayout(workspace);
    rootLayout->setContentsMargins(ThemeConfig::WORKSPACE_MARGIN, ThemeConfig::WORKSPACE_MARGIN,
                                   ThemeConfig::WORKSPACE_MARGIN, ThemeConfig::WORKSPACE_MARGIN);
    rootLayout->setSpacing(ThemeConfig::WORKSPACE_SPACING);

    // ---- Sidebar: program controls + inspector ----
    auto* sidebar = new QWidget(workspace);
    sidebar->setObjectName("SidebarContainer");
    sidebar->setMinimumWidth(ThemeConfig::SIDEBAR_MIN_WIDTH);
    sidebar->setMaximumWidth(ThemeConfig::SIDEBAR_MAX_WIDTH);
    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(ThemeConfig::WORKSPACE_SPACING);

    controlsPanel_ = new ProgramControlsPanel(sidebar);
    inspectorPanel_ = new ValueInspectorPanel(sidebar);
    sidebarLayout->addWidget(controlsPanel_, 1);
    sidebarLayout->addWidget(inspectorPanel_, 0);

    // ---- Main area ----
    auto* mainArea = new QWidget(workspace);
    mainArea->setObjectName("MainAreaContainer");
    auto* mainLayout = new QVBoxLayout(mainArea);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(ThemeConfig::WORKSPACE_SPACING);

    // Top row: [Screen + integrated CPU card] [Registers]
    auto* topRow = new QHBoxLayout();
    topRow->setSpacing(ThemeConfig::WORKSPACE_SPACING);

    screenPanel_ = new ScreenPanel(mainArea);
    screenPanel_->setMaximumHeight(ThemeConfig::TOP_ROW_MAX_HEIGHT);
    statusPanel_ = screenPanel_->statusPanel();

    registerPanel_ = new RegisterGridPanel(mainArea);
    registerPanel_->setFixedWidth(ThemeConfig::REGISTER_PANEL_WIDTH);
    registerPanel_->setMaximumHeight(ThemeConfig::TOP_ROW_MAX_HEIGHT);

    topRow->addWidget(screenPanel_, 1);
    topRow->addWidget(registerPanel_);

    memoryPanel_ = new MemoryMatrixPanel(mainArea);

    mainLayout->addLayout(topRow, ThemeConfig::TOP_ROW_STRETCH);
    mainLayout->addWidget(memoryPanel_, ThemeConfig::MEMORY_ROW_STRETCH);

    rootLayout->addWidget(sidebar);
    rootLayout->addWidget(mainArea, 1);

    // ---- Signals ----
    connect(controlsPanel_, &ProgramControlsPanel::homeRequested, this, &MainWindow::showHomePage);
    connect(controlsPanel_, &ProgramControlsPanel::loadProgramRequested, this, &MainWindow::handleLoadProgram);
    connect(controlsPanel_, &ProgramControlsPanel::stepRequested, this, &MainWindow::handleStep);
    connect(controlsPanel_, &ProgramControlsPanel::runAllRequested, this, &MainWindow::handleRunAll);
    connect(controlsPanel_, &ProgramControlsPanel::haltRequested, this, &MainWindow::handleHalt);
    connect(controlsPanel_, &ProgramControlsPanel::resetRequested, this, &MainWindow::handleReset);

    connect(registerPanel_, &RegisterGridPanel::registerSelected, this, &MainWindow::onRegisterSelected);
    connect(memoryPanel_, &MemoryMatrixPanel::cellSelected, this, &MainWindow::onMemoryCellSelected);

    connect(inspectorPanel_, &ValueInspectorPanel::registerWriteRequested, this, &MainWindow::handleRegisterWrite);
    connect(inspectorPanel_, &ValueInspectorPanel::memoryWriteRequested, this, &MainWindow::handleMemoryWrite);
    connect(inspectorPanel_, &ValueInspectorPanel::invalidValueEntered, this, &MainWindow::handleInvalidInputError);

    return workspace;
}

bool MainWindow::isWorkspaceVisible() const {
    return stack_->currentWidget() == workspace_;
}

void MainWindow::showWorkspace() {
    if (isWorkspaceVisible()) return;

    homePage_->stopIntroAnimation();
    stack_->setCurrentWidget(workspace_);

    // Short fade-in; the workspace is interactive immediately.
    auto* effect = new QGraphicsOpacityEffect(workspace_);
    effect->setOpacity(0.0);
    workspace_->setGraphicsEffect(effect);

    auto* fade = new QPropertyAnimation(effect, "opacity", this);
    fade->setDuration(ThemeConfig::PAGE_FADE_MS);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->setEasingCurve(QEasingCurve::OutCubic);
    connect(fade, &QPropertyAnimation::finished, this, [this]() {
        QTimer::singleShot(0, this, [this]() {
            workspace_->setGraphicsEffect(nullptr);
            // Does nothing if the user already interacted (animator stopped)
            memoryAnimator_->start(ThemeConfig::MEMORY_INTRO_EVENTS,
                                   ThemeConfig::INTRO_EVENT_INTERVAL_MS,
                                   ThemeConfig::INTRO_HIGHLIGHT_MS);
        });
    });
    fade->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::showHomePage() {
    if (!isWorkspaceVisible()) return;

    if (isRunningAsync_) {
        runTimer_->stop();
        isRunningAsync_ = false;
        syncFromSimulator();
    }

    stack_->setCurrentWidget(homePage_);
}

void MainWindow::stopIntroAnimation() {
    memoryAnimator_->stop();   // permanent; never restarted
}

void MainWindow::reportStatus(ExecutionState state, const QString& message) {
    executionState_ = state;
    statusDetail_ = message;
    screenPanel_->appendLog(MachineStatusPanel::stateKey(state), message);
}

void MainWindow::logInfo(const QString& level, const QString& message) {
    statusDetail_ = message;
    screenPanel_->appendLog(level, message);
}

bool MainWindow::parseStartAddress(const QString& rawText, int& outAddress, QString& outError) {
    QString cleaned = rawText.trimmed();
    if (cleaned.startsWith("0x", Qt::CaseInsensitive)) {
        cleaned = cleaned.mid(2).trimmed();
    }
    if (cleaned.isEmpty()) {
        outError = "Load failed: start address is empty.";
        return false;
    }

    bool ok = false;
    const int addr = cleaned.toInt(&ok, 16);
    if (!ok) {
        outError = QString("Load failed: '%1' is not a hexadecimal address.").arg(rawText.trimmed());
        return false;
    }

    // Validation rule comes from the simulator itself
    if (!machine_.getMemory().isValidStart(addr)) {
        outError = QString("Load failed: start address %1 must be even and within 0x00–0xFE.").arg(hex2(addr));
        return false;
    }

    outAddress = addr;
    return true;
}

void MainWindow::handleLoadProgram() {
    if (isRunningAsync_) return;
    stopIntroAnimation();

    int startAddress = 0;
    QString addrError;
    if (!parseStartAddress(controlsPanel_->startAddressText(), startAddress, addrError)) {
        reportStatus(ExecutionState::Error, addrError);
        syncFromSimulator();
        return;
    }

    bool loadedOk = false;
    QString sourceDescription;

    if (controlsPanel_->isUsingFileSource()) {
        const QString filePath = controlsPanel_->selectedFilePath();
        const QFileInfo fi(filePath);
        if (!fi.exists() || !fi.isFile()) {
            reportStatus(ExecutionState::Error, QString("Load failed: file not found '%1'.").arg(filePath));
            syncFromSimulator();
            return;
        }
        loadedOk = machine_.loadProgram(filePath.toStdString(), startAddress);
        sourceDescription = QString("file '%1'").arg(fi.fileName());
    } else {
        const QString manualCode = controlsPanel_->editorText();
        if (manualCode.trimmed().isEmpty()) {
            reportStatus(ExecutionState::Error, "Load failed: the instruction editor is empty.");
            syncFromSimulator();
            return;
        }

        // Hand the raw text to the existing ProgramLoader via a temporary file so it
        // performs all parsing, ';' comment handling and validation itself.
        QTemporaryFile tempFile;
        if (!tempFile.open()) {
            reportStatus(ExecutionState::Error, "Load failed: could not create a temporary file.");
            syncFromSimulator();
            return;
        }
        {
            QTextStream out(&tempFile);
            out << manualCode;
            out.flush();
        }
        tempFile.close();

        loadedOk = machine_.loadProgram(tempFile.fileName().toStdString(), startAddress);
        sourceDescription = "editor";
    }

    if (!loadedOk) {
        reportStatus(ExecutionState::Error,
                     QString("Load failed (%1): tokens must be 2 or 4 hex digits and fit in memory.")
                         .arg(sourceDescription));
        syncFromSimulator();
        return;
    }

    // A previously halted CPU is replaced by a fresh one that keeps the new PC and the registers.
    if (machine_.isHalted()) {
        CPU activeCpu;
        activeCpu.setPC(machine_.getCPU().getPC());
        activeCpu.getRegisters() = machine_.getCPU().getRegisters();
        machine_.getCPU() = activeCpu;
    }
    userHalted_ = false;

    reportStatus(ExecutionState::Ready,
                 QString("Program loaded from %1 at %2.").arg(sourceDescription, hex2(startAddress)));
    syncFromSimulator();
}

void MainWindow::handleStep() {
    if (isRunningAsync_) return;
    stopIntroAnimation();

    if (machine_.isHalted()) {
        const Word ir = machine_.getCPU().getIR();
        if (userHalted_) {
            reportStatus(ExecutionState::Halted, "CPU is halted by user. Load a program or reset to continue.");
        } else if (ir == 0xC000) {
            reportStatus(ExecutionState::Halted, "CPU is halted (C000). Load a program or reset to continue.");
        } else {
            reportStatus(ExecutionState::Error, "CPU stopped after an error. Load a valid program or reset.");
        }
        syncFromSimulator();
        return;
    }

    const Byte pcBefore = machine_.getCPU().getPC();
    machine_.step();

    const Word ir = machine_.getCPU().getIR();
    const Byte pc = machine_.getCPU().getPC();

    if (machine_.isHalted()) {
        if (pcBefore == 255) {
            reportStatus(ExecutionState::Error, "Error: PC reached 0xFF – no room for a 2-byte instruction.");
        } else if (!InstructionFactory::isValid(ir)) {
            reportStatus(ExecutionState::Error,
                         QString("Error: invalid instruction %1 at %2.").arg(hex4(ir), hex2(pcBefore)));
        } else {
            reportStatus(ExecutionState::Halted,
                         QString("Step executed: %1 (HALT) at %2.").arg(hex4(ir), hex2(pcBefore)));
        }
    } else {
        reportStatus(ExecutionState::Ready,
                     QString("Step executed: %1  %2  → PC %3")
                         .arg(hex4(ir), MachineStatusPanel::describeInstruction(ir), hex2(pc)));
    }

    syncFromSimulator();
}

void MainWindow::handleRunAll() {
    if (isRunningAsync_) return;
    stopIntroAnimation();

    if (machine_.isHalted()) {
        handleStep();   // reports why nothing can run
        return;
    }

    isRunningAsync_ = true;
    runSteps_ = 0;
    reportStatus(ExecutionState::Running,
                 QString("Run All started at PC %1.").arg(hex2(machine_.getCPU().getPC())));
    syncFromSimulator();
    runRefreshClock_.start();
    runTimer_->start();
}

void MainWindow::onRunTick() {
    int batch = 0;
    while (batch < RUN_BATCH_STEPS && !machine_.isHalted() && runSteps_ < RUN_MAX_STEPS) {
        lastPcBeforeStep_ = machine_.getCPU().getPC();
        machine_.step();
        ++runSteps_;
        ++batch;
    }

    if (machine_.isHalted() || runSteps_ >= RUN_MAX_STEPS) {
        finishRunAll(false);
        return;
    }

    if (runRefreshClock_.elapsed() >= RUN_UI_REFRESH_MS) {
        syncFromSimulator();
        runRefreshClock_.restart();
    }
}

void MainWindow::finishRunAll(bool haltedByUser) {
    runTimer_->stop();
    isRunningAsync_ = false;

    const Word ir = machine_.getCPU().getIR();
    const Byte pc = machine_.getCPU().getPC();

    if (haltedByUser) {
        reportStatus(ExecutionState::Halted,
                     QString("Run All halted by user after %1 steps at PC %2.").arg(runSteps_).arg(hex2(pc)));
    } else if (!machine_.isHalted()) {
        reportStatus(ExecutionState::Error,
                     QString("Error: stopped after %1 steps without halting (possible infinite loop at PC %2).")
                         .arg(RUN_MAX_STEPS).arg(hex2(pc)));
    } else if (lastPcBeforeStep_ == 255) {
        reportStatus(ExecutionState::Error, "Error: PC reached 0xFF – no room for a 2-byte instruction.");
    } else if (!InstructionFactory::isValid(ir)) {
        reportStatus(ExecutionState::Error,
                     QString("Error: invalid instruction %1 at %2.").arg(hex4(ir), hex2(lastPcBeforeStep_)));
    } else {
        reportStatus(ExecutionState::Halted,
                     QString("Run All finished: HALT (C000) after %1 steps.").arg(runSteps_));
    }

    syncFromSimulator();
    emit runAllFinished();
}

void MainWindow::handleHalt() {
    stopIntroAnimation();

    if (machine_.isHalted()) {
        logInfo("info", "CPU is already halted.");
        syncFromSimulator();
        return;
    }

    machine_.getCPU().halt();   // simulator's own halt
    userHalted_ = true;

    if (isRunningAsync_) {
        finishRunAll(true);
        return;
    }

    reportStatus(ExecutionState::Halted,
                 QString("Halted by user at PC %1.").arg(hex2(machine_.getCPU().getPC())));
    syncFromSimulator();
}

void MainWindow::handleReset() {
    if (isRunningAsync_) return;
    stopIntroAnimation();

    machine_.reset();
    userHalted_ = false;
    reportStatus(ExecutionState::Ready, "Machine reset: PC, IR, registers, memory and screen cleared.");
    syncFromSimulator();
}

void MainWindow::handleRegisterWrite(int regIndex, Byte value) {
    if (isRunningAsync_) return;
    stopIntroAnimation();

    machine_.getCPU().getRegisters().set(regIndex, value);
    if (executionState_ == ExecutionState::Error) {
        executionState_ = machine_.isHalted() ? ExecutionState::Halted : ExecutionState::Ready;
    }
    logInfo("edit", QString("R%1 set to %2 (%3).")
                        .arg(QString::number(regIndex, 16).toUpper(), hex2(value))
                        .arg(static_cast<int>(value)));
    syncFromSimulator();
}

void MainWindow::handleMemoryWrite(int address, Byte value) {
    if (isRunningAsync_) return;
    stopIntroAnimation();

    machine_.getMemory().write(address, value);
    if (executionState_ == ExecutionState::Error) {
        executionState_ = machine_.isHalted() ? ExecutionState::Halted : ExecutionState::Ready;
    }
    logInfo("edit", QString("Memory [%1] set to %2 (%3).")
                        .arg(hex2(address), hex2(value))
                        .arg(static_cast<int>(value)));
    syncFromSimulator();
}

void MainWindow::handleInvalidInputError(const QString& message) {
    stopIntroAnimation();
    logInfo("error", message);
}

void MainWindow::onRegisterSelected(int regIndex) {
    stopIntroAnimation();
    memoryPanel_->setSelectedAddress(-1);
    inspectorPanel_->inspectRegister(regIndex, machine_.getCPU().getRegisters().get(regIndex));
}

void MainWindow::onMemoryCellSelected(int address) {
    stopIntroAnimation();
    registerPanel_->setSelectedRegister(-1);
    inspectorPanel_->inspectMemoryCell(address, machine_.getMemory().read(address));
}

void MainWindow::syncFromSimulator() {
    CPU& cpu = machine_.getCPU();
    Memory& memory = machine_.getMemory();
    Screen& screen = machine_.getScreen();

    statusPanel_->updateStatus(cpu.getPC(), cpu.getIR(), executionState_);
    registerPanel_->updateRegisters(cpu.getRegisters());
    memoryPanel_->updateMemory(memory, cpu.getPC());
    screenPanel_->updateScreen(screen.getContent());

    controlsPanel_->setExecutionControls(isRunningAsync_, machine_.isHalted());
    inspectorPanel_->setEditingEnabled(!isRunningAsync_);

    if (inspectorPanel_->currentTargetType() == SelectionTargetType::Register) {
        inspectorPanel_->refreshCurrentValue(cpu.getRegisters().get(inspectorPanel_->currentTargetIndex()));
    } else if (inspectorPanel_->currentTargetType() == SelectionTargetType::MemoryCell) {
        inspectorPanel_->refreshCurrentValue(memory.read(inspectorPanel_->currentTargetIndex()));
    }
}
