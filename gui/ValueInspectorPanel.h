#ifndef VALUE_INSPECTOR_PANEL_H
#define VALUE_INSPECTOR_PANEL_H

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "../VoleMachine.h"

enum class SelectionTargetType {
    None,
    Register,
    MemoryCell
};

class ValueInspectorPanel : public QFrame {
    Q_OBJECT

public:
    explicit ValueInspectorPanel(QWidget* parent = nullptr);

    void inspectRegister(int regIndex, Byte value);
    void inspectMemoryCell(int address, Byte value);
    void refreshCurrentValue(Byte value);

    SelectionTargetType currentTargetType() const { return targetType_; }
    int currentTargetIndex() const { return targetIndex_; }

    void setEditingEnabled(bool enabled);

signals:
    void registerWriteRequested(int regIndex, Byte value);
    void memoryWriteRequested(int address, Byte value);
    void invalidValueEntered(const QString& errorMessage);

private slots:
    void onApplyClicked();

private:
    SelectionTargetType targetType_ = SelectionTargetType::None;
    int targetIndex_ = -1;
    Byte currentValue_ = 0;

    QLabel* targetBadge_ = nullptr;
    QLabel* targetTitleLabel_ = nullptr;
    QLabel* hexValueLabel_ = nullptr;
    QLabel* unsignedDecLabel_ = nullptr;
    QLabel* signedDecLabel_ = nullptr;
    QLabel* binaryValueLabel_ = nullptr;
    QLabel* asciiValueLabel_ = nullptr;

    QLineEdit* hexInputEdit_ = nullptr;
    QPushButton* applyButton_ = nullptr;

    void updateDisplayFields();
};

#endif // VALUE_INSPECTOR_PANEL_H
