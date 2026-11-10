#include "ValueInspectorPanel.h"
#include "ThemeConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

ValueInspectorPanel::ValueInspectorPanel(QWidget* parent)
    : QFrame(parent) {
    setObjectName("ValueInspectorPanel");
    setProperty("panelRole", "card");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(ThemeConfig::PANEL_PADDING, ThemeConfig::PANEL_PADDING,
                                   ThemeConfig::PANEL_PADDING, ThemeConfig::PANEL_PADDING);
    mainLayout->setSpacing(8);

    // Header Row
    auto* headerLayout = new QHBoxLayout();
    auto* titleLabel = new QLabel("SELECTED VALUE INSPECTOR", this);
    titleLabel->setObjectName("SectionTitle");

    targetBadge_ = new QLabel("MEMORY", this);
    targetBadge_->setObjectName("InspectorTargetBadge");
    targetBadge_->setProperty("targetType", "memory");

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(targetBadge_);
    mainLayout->addLayout(headerLayout);

    // Selected Identifier Banner
    targetTitleLabel_ = new QLabel("Memory Cell [0x00]", this);
    targetTitleLabel_->setObjectName("InspectorTargetTitle");
    mainLayout->addWidget(targetTitleLabel_);

    // Value Representations Grid
    auto* infoFrame = new QFrame(this);
    infoFrame->setObjectName("InspectorInfoBox");
    auto* grid = new QGridLayout(infoFrame);
    grid->setContentsMargins(10, 8, 10, 8);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(5);

    auto addRow = [&](int row, const QString& labelText, QLabel*& valLabel) {
        auto* keyLbl = new QLabel(labelText, infoFrame);
        keyLbl->setObjectName("InspectorKeyLabel");
        valLabel = new QLabel("—", infoFrame);
        valLabel->setObjectName("InspectorValLabel");
        valLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        grid->addWidget(keyLbl, row, 0);
        grid->addWidget(valLabel, row, 1);
    };

    addRow(0, "Hexadecimal:", hexValueLabel_);
    addRow(1, "Unsigned Decimal:", unsignedDecLabel_);
    addRow(2, "Signed (2's Comp):", signedDecLabel_);
    addRow(3, "Binary (8-Bit):", binaryValueLabel_);
    addRow(4, "ASCII Character:", asciiValueLabel_);

    mainLayout->addWidget(infoFrame);

    // Overwrite Value Controls
    auto* editLabel = new QLabel("Overwrite Selected Value (Hex 00..FF):", this);
    editLabel->setObjectName("FieldLabel");
    mainLayout->addWidget(editLabel);

    auto* editRowLayout = new QHBoxLayout();
    editRowLayout->setSpacing(8);

    hexInputEdit_ = new QLineEdit(this);
    hexInputEdit_->setObjectName("InspectorHexInput");
    hexInputEdit_->setPlaceholderText("e.g. 0x2A or FF");
    hexInputEdit_->setAlignment(Qt::AlignCenter);

    applyButton_ = new QPushButton("Overwrite Value", this);
    applyButton_->setObjectName("OverwriteButton");
    applyButton_->setCursor(Qt::PointingHandCursor);
    applyButton_->setMinimumHeight(32);

    editRowLayout->addWidget(hexInputEdit_, 1);
    editRowLayout->addWidget(applyButton_);
    mainLayout->addLayout(editRowLayout);

    connect(applyButton_, &QPushButton::clicked, this, &ValueInspectorPanel::onApplyClicked);
    connect(hexInputEdit_, &QLineEdit::returnPressed, this, &ValueInspectorPanel::onApplyClicked);

    inspectMemoryCell(0, 0);
}

void ValueInspectorPanel::inspectRegister(int regIndex, Byte value) {
    targetType_ = SelectionTargetType::Register;
    targetIndex_ = regIndex;
    currentValue_ = value;
    updateDisplayFields();
    hexInputEdit_->setText("0x" + QString("%1").arg(value, 2, 16, QChar('0')).toUpper());
}

void ValueInspectorPanel::inspectMemoryCell(int address, Byte value) {
    targetType_ = SelectionTargetType::MemoryCell;
    targetIndex_ = address;
    currentValue_ = value;
    updateDisplayFields();
    hexInputEdit_->setText("0x" + QString("%1").arg(value, 2, 16, QChar('0')).toUpper());
}

void ValueInspectorPanel::refreshCurrentValue(Byte value) {
    currentValue_ = value;
    updateDisplayFields();
    if (!hexInputEdit_->hasFocus()) {
        hexInputEdit_->setText("0x" + QString("%1").arg(value, 2, 16, QChar('0')).toUpper());
    }
}

void ValueInspectorPanel::setEditingEnabled(bool enabled) {
    hexInputEdit_->setEnabled(enabled);
    applyButton_->setEnabled(enabled);
}

void ValueInspectorPanel::updateDisplayFields() {
    if (targetType_ == SelectionTargetType::Register) {
        QString regHex = QString::number(targetIndex_, 16).toUpper();
        targetBadge_->setText(QString("REGISTER R%1").arg(regHex));
        targetBadge_->setProperty("targetType", "register");
        targetTitleLabel_->setText(QString("General Purpose Register R%1").arg(regHex));
    } else if (targetType_ == SelectionTargetType::MemoryCell) {
        QString addrHex = QString("%1").arg(targetIndex_, 2, 16, QChar('0')).toUpper();
        targetBadge_->setText(QString("MEM [0x%1]").arg(addrHex));
        targetBadge_->setProperty("targetType", "memory");
        targetTitleLabel_->setText(QString("Memory Address 0x%1 (Dec: %2)").arg(addrHex).arg(targetIndex_));
    } else {
        targetBadge_->setText("NONE");
        targetBadge_->setProperty("targetType", "none");
        targetTitleLabel_->setText("Select a register or memory cell");
        return;
    }
    ThemeConfig::refreshWidgetStyle(targetBadge_);

    QString hexStr = "0x" + QString("%1").arg(currentValue_, 2, 16, QChar('0')).toUpper();
    int unsignedVal = static_cast<int>(currentValue_);
    int signedVal = static_cast<signed char>(currentValue_);
    QString binRaw = QString("%1").arg(currentValue_, 8, 2, QChar('0'));
    QString binFormatted = binRaw.left(4) + " " + binRaw.mid(4);

    hexValueLabel_->setText(hexStr);
    unsignedDecLabel_->setText(QString::number(unsignedVal));
    signedDecLabel_->setText(signedVal > 0 ? QString("+%1").arg(signedVal) : QString::number(signedVal));
    binaryValueLabel_->setText(binFormatted);

    if (currentValue_ >= 32 && currentValue_ <= 126) {
        asciiValueLabel_->setText(QString("'%1'").arg(QChar(currentValue_)));
    } else {
        asciiValueLabel_->setText(QString("Non-printable (%1)").arg(hexStr));
    }
}

void ValueInspectorPanel::onApplyClicked() {
    if (targetType_ == SelectionTargetType::None || targetIndex_ < 0) {
        return;
    }

    QString raw = hexInputEdit_->text().trimmed();
    if (raw.startsWith("0x", Qt::CaseInsensitive)) {
        raw = raw.mid(2).trimmed();
    }

    if (raw.isEmpty() || raw.length() > 2) {
        emit invalidValueEntered("Invalid byte value: enter 1 or 2 hexadecimal digits (00 to FF).");
        return;
    }

    bool ok = false;
    int parsed = raw.toInt(&ok, 16);
    if (!ok || parsed < 0 || parsed > 255) {
        emit invalidValueEntered(QString("Invalid hexadecimal value '%1': expected 0x00 to 0xFF.").arg(hexInputEdit_->text().trimmed()));
        return;
    }

    Byte newValue = static_cast<Byte>(parsed);
    if (targetType_ == SelectionTargetType::Register) {
        emit registerWriteRequested(targetIndex_, newValue);
    } else if (targetType_ == SelectionTargetType::MemoryCell) {
        emit memoryWriteRequested(targetIndex_, newValue);
    }
}
