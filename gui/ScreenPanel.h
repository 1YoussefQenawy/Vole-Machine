#ifndef SCREEN_PANEL_H
#define SCREEN_PANEL_H

#include <QFrame>
#include <QLabel>
#include <QPlainTextEdit>
#include <string>

class QScrollArea;
class QVBoxLayout;
class MachineStatusPanel;

// Unified Screen + CPU card:
//  - Left: simulated display with compact output area (~0.4x previous height),
//          unchanged execution log height, and bottom-left HEX/DEC indicators.
//  - Right: merged compact CPU information sub-card (status badge, PC, IR).
class ScreenPanel : public QFrame {
    Q_OBJECT

public:
    explicit ScreenPanel(QWidget* parent = nullptr);

    void updateScreen(const std::string& screenContent);
    void appendLog(const QString& level, const QString& message);

    MachineStatusPanel* statusPanel() const { return statusPanel_; }
    int logEntryCount() const { return logCount_; }
    QString lastLogMessage() const { return lastLog_; }
    QString outputText() const { return outputView_->toPlainText(); }
    QPlainTextEdit* outputView() const { return outputView_; }
    QScrollArea* logArea() const { return logArea_; }

private:
    QLabel* byteCountBadge_ = nullptr;
    QPlainTextEdit* outputView_ = nullptr;
    QScrollArea* logArea_ = nullptr;
    QWidget* logContainer_ = nullptr;
    QVBoxLayout* logLayout_ = nullptr;
    QLabel* hexLabel_ = nullptr;
    QLabel* decLabel_ = nullptr;
    MachineStatusPanel* statusPanel_ = nullptr;

    std::string lastContent_;
    bool initialized_ = false;
    int logCount_ = 0;
    QString lastLog_;
};

#endif // SCREEN_PANEL_H
