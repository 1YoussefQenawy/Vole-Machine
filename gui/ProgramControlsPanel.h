#ifndef PROGRAM_CONTROLS_PANEL_H
#define PROGRAM_CONTROLS_PANEL_H

#include <QFrame>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>

class ProgramControlsPanel : public QFrame {
    Q_OBJECT

public:
    explicit ProgramControlsPanel(QWidget* parent = nullptr);

    QString selectedFilePath() const;
    QString editorText() const;
    QString startAddressText() const;
    bool isUsingFileSource() const;

    // running: Run All in progress (only Halt stays usable)
    // machineHalted: CPU is halted (Halt has nothing to stop)
    void setExecutionControls(bool running, bool machineHalted);
    void setProgramText(const QString& text, const QString& filePath = QString());
    void setStartAddressText(const QString& text);

    QPushButton* haltButton() const { return haltButton_; }
    QPushButton* homeButton() const { return homeButton_; }

signals:
    void homeRequested();
    void loadProgramRequested();
    void stepRequested();
    void runAllRequested();
    void haltRequested();
    void resetRequested();

private slots:
    void onBrowseClicked();
    void onClearSourceClicked();
    void onEditorTextChanged();
    void onFilePathEdited(const QString& path);

private:
    QPushButton* homeButton_ = nullptr;
    QLineEdit* filePathEdit_ = nullptr;
    QPushButton* browseButton_ = nullptr;
    QPushButton* clearSourceButton_ = nullptr;
    QLabel* sourceModeBadge_ = nullptr;

    QLineEdit* startAddressEdit_ = nullptr;
    QPlainTextEdit* programEditor_ = nullptr;
    QPushButton* loadProgramButton_ = nullptr;

    QPushButton* runAllButton_ = nullptr;
    QPushButton* stepButton_ = nullptr;
    QPushButton* haltButton_ = nullptr;
    QPushButton* resetButton_ = nullptr;

    bool usingFileSource_ = false;
    bool suppressEditorSignal_ = false;

    void updateSourceBadge();
};

#endif // PROGRAM_CONTROLS_PANEL_H
