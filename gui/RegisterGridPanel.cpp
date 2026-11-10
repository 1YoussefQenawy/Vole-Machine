#include "RegisterGridPanel.h"
#include "ThemeConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMouseEvent>

RegisterCellWidget::RegisterCellWidget(int regIndex, QWidget* parent)
    : QFrame(parent), regIndex_(regIndex) {
    setObjectName("RegisterCell");
    setProperty("selected", false);
    setProperty("nonZero", false);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(7, 2, 8, 2);
    layout->setSpacing(4);

    nameLabel_ = new QLabel(QString("R%1").arg(regIndex, 1, 16).toUpper(), this);
    nameLabel_->setObjectName("RegisterNameLabel");
    nameLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    nameLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);

    hexValueLabel_ = new QLabel("00", this);
    hexValueLabel_->setObjectName("RegisterHexValue");
    hexValueLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hexValueLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);

    layout->addWidget(nameLabel_);
    layout->addWidget(hexValueLabel_, 1);

    setValue(0);
}

void RegisterCellWidget::setValue(Byte value) {
    if (initialized_ && currentValue_ == value) {
        return;
    }
    const bool nonZeroChanged = (!initialized_) || ((currentValue_ != 0) != (value != 0));
    initialized_ = true;
    currentValue_ = value;

    const QString hexStr = QString("%1").arg(value, 2, 16, QChar('0')).toUpper();
    const QString binStr = QString("%1").arg(value, 8, 2, QChar('0'));
    hexValueLabel_->setText(hexStr);

    const int signedVal = static_cast<signed char>(value);
    setToolTip(QString("Register R%1\nHex: 0x%2\nUnsigned: %3\nSigned: %4\nBinary: %5")
               .arg(QString::number(regIndex_, 16).toUpper())
               .arg(hexStr)
               .arg(static_cast<int>(value))
               .arg(signedVal)
               .arg(binStr));

    if (nonZeroChanged) {
        setProperty("nonZero", value != 0);
        ThemeConfig::refreshWidgetStyle(this);
    }
}

void RegisterCellWidget::setSelected(bool selected) {
    if (selected_ == selected) return;
    selected_ = selected;
    setProperty("selected", selected);
    ThemeConfig::refreshWidgetStyle(this);
}

void RegisterCellWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(regIndex_);
    }
    QFrame::mousePressEvent(event);
}

RegisterGridPanel::RegisterGridPanel(QWidget* parent)
    : QFrame(parent) {
    setObjectName("RegisterGridPanel");
    setProperty("panelRole", "card");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(ThemeConfig::PANEL_PADDING_COMPACT, ThemeConfig::PANEL_PADDING_COMPACT,
                                   ThemeConfig::PANEL_PADDING_COMPACT, ThemeConfig::PANEL_PADDING_COMPACT);
    mainLayout->setSpacing(6);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    auto* titleLabel = new QLabel("REGISTERS", this);
    titleLabel->setObjectName("SectionTitle");
    auto* subtitleBadge = new QLabel("R0–RF", this);
    subtitleBadge->setObjectName("PanelMetaBadge");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(subtitleBadge);
    mainLayout->addLayout(headerLayout);

    auto* gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(ThemeConfig::REGISTER_GRID_SPACING);

    for (int r = 0; r < ThemeConfig::REGISTER_GRID_ROWS; ++r) {
        gridLayout->setRowStretch(r, 1);
    }
    for (int c = 0; c < ThemeConfig::REGISTER_GRID_COLS; ++c) {
        gridLayout->setColumnStretch(c, 1);
    }

    for (int i = 0; i < 16; ++i) {
        const int row = i / ThemeConfig::REGISTER_GRID_COLS;
        const int col = i % ThemeConfig::REGISTER_GRID_COLS;
        cells_[i] = new RegisterCellWidget(i, this);
        gridLayout->addWidget(cells_[i], row, col);

        connect(cells_[i], &RegisterCellWidget::clicked, this, [this](int regIdx) {
            setSelectedRegister(regIdx);
            emit registerSelected(regIdx);
        });
    }

    mainLayout->addLayout(gridLayout, 1);
}

void RegisterGridPanel::updateRegisters(RegisterFile& regFile) {
    for (int i = 0; i < 16; ++i) {
        cells_[i]->setValue(regFile.get(i));
    }
}

void RegisterGridPanel::setSelectedRegister(int index) {
    if (selectedIndex_ >= 0 && selectedIndex_ < 16) {
        cells_[selectedIndex_]->setSelected(false);
    }
    selectedIndex_ = index;
    if (selectedIndex_ >= 0 && selectedIndex_ < 16) {
        cells_[selectedIndex_]->setSelected(true);
    }
}
