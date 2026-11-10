#ifndef MEMORY_MATRIX_PANEL_H
#define MEMORY_MATRIX_PANEL_H

#include <QFrame>
#include <QLabel>
#include <QVector>
#include <array>
#include "../VoleMachine.h"

class MemoryCellWidget : public QFrame {
    Q_OBJECT

public:
    explicit MemoryCellWidget(int address, QWidget* parent = nullptr);

    void setState(Byte value, bool isPC, bool isPCNext, bool isSelected);
    void setSelected(bool selected);
    int address() const { return address_; }

signals:
    void clicked(int address);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    int address_;
    Byte value_ = 0;
    bool isPC_ = false;
    bool isPCNext_ = false;
    bool isSelected_ = false;
    bool initialized_ = false;

    QLabel* addrLabel_ = nullptr;
    QLabel* valLabel_ = nullptr;
};

class MemoryMatrixPanel : public QFrame {
    Q_OBJECT

public:
    explicit MemoryMatrixPanel(QWidget* parent = nullptr);

    void updateMemory(Memory& memory, Byte pc);
    void setSelectedAddress(int address);
    int selectedAddress() const { return selectedAddress_; }
    QVector<QWidget*> cellWidgets() const;

signals:
    void cellSelected(int address);

private:
    std::array<MemoryCellWidget*, 256> cells_{};
    Byte currentPC_ = 0;
    int selectedAddress_ = -1;
};

#endif // MEMORY_MATRIX_PANEL_H
