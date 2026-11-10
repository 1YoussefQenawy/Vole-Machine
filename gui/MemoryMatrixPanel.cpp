#include "MemoryMatrixPanel.h"
#include "ThemeConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMouseEvent>

MemoryCellWidget::MemoryCellWidget(int address, QWidget* parent)
    : QFrame(parent), address_(address) {
    setObjectName("MemoryCell");
    setProperty("selected", false);
    setProperty("isPC", false);
    setProperty("isPCNext", false);
    setProperty("nonZero", false);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 2, 7, 2);
    layout->setSpacing(4);

    QString addrHex = QString("%1").arg(address, 2, 16, QChar('0')).toUpper();
    addrLabel_ = new QLabel(addrHex, this);
    addrLabel_->setObjectName("MemoryCellAddr");
    addrLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    addrLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);

    valLabel_ = new QLabel("00", this);
    valLabel_->setObjectName("MemoryCellVal");
    valLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    valLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);

    layout->addWidget(addrLabel_);
    layout->addWidget(valLabel_, 1);

    setState(0, false, false, false);
}

void MemoryCellWidget::setState(Byte value, bool isPC, bool isPCNext, bool isSelected) {
    bool nonZero = (value != 0);
    bool wasNonZero = (value_ != 0);

    bool styleChanged = (!initialized_) ||
                        (nonZero != wasNonZero) ||
                        (isPC != isPC_) ||
                        (isPCNext != isPCNext_) ||
                        (isSelected != isSelected_);

    if (!initialized_ || value != value_) {
        QString valHex = QString("%1").arg(value, 2, 16, QChar('0')).toUpper();
        valLabel_->setText(valHex);

        QString addrHex = QString("%1").arg(address_, 2, 16, QChar('0')).toUpper();
        QString binStr = QString("%1").arg(value, 8, 2, QChar('0'));
        int signedVal = static_cast<signed char>(value);
        setToolTip(QString("Memory Address: 0x%1 (Dec: %2)\nHex Value: 0x%3\nUnsigned Dec: %4\nSigned Dec: %5\nBinary: %6\nClick to inspect or overwrite")
                   .arg(addrHex)
                   .arg(address_)
                   .arg(valHex)
                   .arg(static_cast<int>(value))
                   .arg(signedVal)
                   .arg(binStr));
    }

    initialized_ = true;
    value_ = value;
    isPC_ = isPC;
    isPCNext_ = isPCNext;
    isSelected_ = isSelected;

    if (styleChanged) {
        setProperty("nonZero", nonZero);
        setProperty("isPC", isPC);
        setProperty("isPCNext", isPCNext);
        setProperty("selected", isSelected);
        ThemeConfig::refreshWidgetStyle(this);
    }
}

void MemoryCellWidget::setSelected(bool selected) {
    if (isSelected_ == selected) return;
    isSelected_ = selected;
    setProperty("selected", selected);
    ThemeConfig::refreshWidgetStyle(this);
}

void MemoryCellWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(address_);
    }
    QFrame::mousePressEvent(event);
}

MemoryMatrixPanel::MemoryMatrixPanel(QWidget* parent)
    : QFrame(parent) {
    setObjectName("MemoryMatrixPanel");
    setProperty("panelRole", "card");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(ThemeConfig::PANEL_PADDING, ThemeConfig::PANEL_PADDING,
                                   ThemeConfig::PANEL_PADDING, ThemeConfig::PANEL_PADDING);
    mainLayout->setSpacing(ThemeConfig::PANEL_SPACING);

    // Header Row with Legend
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(14);

    auto* titleLabel = new QLabel("MAIN MEMORY", this);
    titleLabel->setObjectName("SectionTitle");

    auto* metaLabel = new QLabel("256 BYTES · 0x00–0xFF", this);
    metaLabel->setObjectName("PanelMetaBadge");

    auto* pcLegend = new QLabel("■ PC", this);
    pcLegend->setObjectName("LegendBadgePC");

    auto* nonZeroLegend = new QLabel("■ Non-zero", this);
    nonZeroLegend->setObjectName("LegendBadgeNonZero");

    auto* selectedLegend = new QLabel("■ Selected", this);
    selectedLegend->setObjectName("LegendBadgeSelected");

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(metaLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(pcLegend);
    headerLayout->addWidget(nonZeroLegend);
    headerLayout->addWidget(selectedLegend);
    mainLayout->addLayout(headerLayout);

    // 16 x 16 Grid with Row & Column Headers
    auto* gridLayout = new QGridLayout();
    gridLayout->setSpacing(ThemeConfig::MEMORY_MATRIX_SPACING);
    gridLayout->setContentsMargins(0, 2, 0, 0);

    // Top-left corner label
    auto* cornerLabel = new QLabel("ADDR", this);
    cornerLabel->setObjectName("MatrixHeaderCorner");
    cornerLabel->setAlignment(Qt::AlignCenter);
    gridLayout->addWidget(cornerLabel, 0, 0);

    // Column Headers: +0 .. +F
    for (int col = 0; col < ThemeConfig::MEMORY_MATRIX_COLS; ++col) {
        auto* colHeader = new QLabel(QString("+%1").arg(col, 1, 16).toUpper(), this);
        colHeader->setObjectName("MatrixColHeader");
        colHeader->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(colHeader, 0, col + 1);
        gridLayout->setColumnStretch(col + 1, 1);
    }

    // Row Headers (0x00 .. 0xF0) + 256 Memory Cells
    for (int row = 0; row < ThemeConfig::MEMORY_MATRIX_ROWS; ++row) {
        auto* rowHeader = new QLabel("0x" + QString("%10").arg(row, 1, 16).toUpper(), this);
        rowHeader->setObjectName("MatrixRowHeader");
        rowHeader->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(rowHeader, row + 1, 0);
        gridLayout->setRowStretch(row + 1, 1);

        for (int col = 0; col < ThemeConfig::MEMORY_MATRIX_COLS; ++col) {
            int addr = row * 16 + col;
            cells_[addr] = new MemoryCellWidget(addr, this);
            gridLayout->addWidget(cells_[addr], row + 1, col + 1);

            connect(cells_[addr], &MemoryCellWidget::clicked, this, [this](int clickedAddr) {
                setSelectedAddress(clickedAddr);
                emit cellSelected(clickedAddr);
            });
        }
    }

    mainLayout->addLayout(gridLayout, 1);
}

void MemoryMatrixPanel::updateMemory(Memory& memory, Byte pc) {
    currentPC_ = pc;
    int pcAddr = static_cast<int>(pc);
    int pcNextAddr = (pcAddr < 255) ? (pcAddr + 1) : -1;

    for (int addr = 0; addr < 256; ++addr) {
        Byte val = memory.read(addr);
        bool isPC = (addr == pcAddr);
        bool isPCNext = (addr == pcNextAddr);
        bool isSelected = (addr == selectedAddress_);
        cells_[addr]->setState(val, isPC, isPCNext, isSelected);
    }
}

void MemoryMatrixPanel::setSelectedAddress(int address) {
    int prev = selectedAddress_;
    selectedAddress_ = address;

    if (prev >= 0 && prev < 256) {
        cells_[prev]->setSelected(false);
    }
    if (selectedAddress_ >= 0 && selectedAddress_ < 256) {
        cells_[selectedAddress_]->setSelected(true);
    }
}

QVector<QWidget*> MemoryMatrixPanel::cellWidgets() const {
    QVector<QWidget*> widgets;
    widgets.reserve(256);
    for (MemoryCellWidget* cell : cells_) {
        widgets.append(cell);
    }
    return widgets;
}
