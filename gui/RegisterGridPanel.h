#ifndef REGISTER_GRID_PANEL_H
#define REGISTER_GRID_PANEL_H

#include <QFrame>
#include <QLabel>
#include <array>
#include "../VoleMachine.h"

class RegisterCellWidget : public QFrame {
    Q_OBJECT

public:
    explicit RegisterCellWidget(int regIndex, QWidget* parent = nullptr);

    void setValue(Byte value);
    void setSelected(bool selected);
    int registerIndex() const { return regIndex_; }

signals:
    void clicked(int regIndex);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    int regIndex_;
    Byte currentValue_ = 0;
    bool selected_ = false;
    bool initialized_ = false;

    QLabel* nameLabel_ = nullptr;
    QLabel* hexValueLabel_ = nullptr;
};

class RegisterGridPanel : public QFrame {
    Q_OBJECT

public:
    explicit RegisterGridPanel(QWidget* parent = nullptr);

    void updateRegisters(RegisterFile& regFile);
    void setSelectedRegister(int index);
    int selectedRegister() const { return selectedIndex_; }
    RegisterCellWidget* cellWidget(int index) const { return cells_.at(index); }

signals:
    void registerSelected(int regIndex);

private:
    std::array<RegisterCellWidget*, 16> cells_{};
    int selectedIndex_ = -1;
};

#endif // REGISTER_GRID_PANEL_H
