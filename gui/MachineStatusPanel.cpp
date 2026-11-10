#include "MachineStatusPanel.h"
#include "ThemeConfig.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

MachineStatusPanel::MachineStatusPanel(QWidget* parent)
    : QFrame(parent) {
    setObjectName("CpuIntegratedCard");
    setFixedWidth(ThemeConfig::CPU_SUBCARD_WIDTH);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(7, 6, 7, 6);
    mainLayout->setSpacing(3);

    // Status badge at top of the merged CPU card
    statusBadgeLabel_ = new QLabel(this);
    statusBadgeLabel_->setObjectName("ExecutionStatusBadge");
    statusBadgeLabel_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusBadgeLabel_);

    mainLayout->addStretch(1);

    // PC section (inside the same merged card)
    auto* pcHeaderRow = new QHBoxLayout();
    pcHeaderRow->setContentsMargins(0, 0, 0, 0);
    pcHeaderRow->setSpacing(2);
    auto* pcTitle = new QLabel("PC", this);
    pcTitle->setObjectName("MetricLabel");
    pcDecLabel_ = new QLabel("Dec 0", this);
    pcDecLabel_->setObjectName("MetricSubValue");
    pcDecLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    pcHeaderRow->addWidget(pcTitle);
    pcHeaderRow->addStretch(1);
    pcHeaderRow->addWidget(pcDecLabel_);
    mainLayout->addLayout(pcHeaderRow);

    pcHexLabel_ = new QLabel("0x00", this);
    pcHexLabel_->setObjectName("MetricPrimaryValue");
    mainLayout->addWidget(pcHexLabel_);

    mainLayout->addStretch(1);

    // Subtle divider between PC and IR inside the merged card
    auto* divider = new QFrame(this);
    divider->setObjectName("PanelDivider");
    divider->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(divider);

    mainLayout->addStretch(1);

    // IR section (inside the same merged card)
    auto* irTitle = new QLabel("IR", this);
    irTitle->setObjectName("MetricLabel");
    mainLayout->addWidget(irTitle);

    irHexLabel_ = new QLabel("0x0000", this);
    irHexLabel_->setObjectName("MetricPrimaryValue");
    mainLayout->addWidget(irHexLabel_);

    irDecodedLabel_ = new QLabel("—", this);
    irDecodedLabel_->setObjectName("MetricSubValue");
    irDecodedLabel_->setWordWrap(true);
    mainLayout->addWidget(irDecodedLabel_);

    mainLayout->addStretch(1);

    updateStatus(0, 0, ExecutionState::Ready);
}

QString MachineStatusPanel::stateKey(ExecutionState state) {
    switch (state) {
        case ExecutionState::Ready:   return "ready";
        case ExecutionState::Running: return "running";
        case ExecutionState::Halted:  return "halted";
        case ExecutionState::Error:   return "error";
    }
    return "ready";
}

void MachineStatusPanel::updateStatus(Byte pc, Word ir, ExecutionState state) {
    const QString pcHex = "0x" + QString("%1").arg(pc, 2, 16, QChar('0')).toUpper();
    const QString pcDec = QString("Dec %1").arg(static_cast<int>(pc));
    pcHexLabel_->setText(pcHex);
    pcDecLabel_->setText(pcDec);

    const QString irHex = "0x" + QString("%1").arg(ir, 4, 16, QChar('0')).toUpper();
    const QString irDesc = describeInstruction(ir);
    irHexLabel_->setText(irHex);
    irDecodedLabel_->setText(irDesc);

    setToolTip(QString("CPU Status\nProgram Counter: %1 (%2)\nInstruction Register: %3 (%4)")
                   .arg(pcHex, pcDec, irHex, irDesc));

    QString badgeText;
    switch (state) {
        case ExecutionState::Ready:   badgeText = "● READY";   break;
        case ExecutionState::Running: badgeText = "◉ RUNNING"; break;
        case ExecutionState::Halted:  badgeText = "■ HALTED";  break;
        case ExecutionState::Error:   badgeText = "▲ ERROR";   break;
    }

    const QString key = stateKey(state);
    statusBadgeLabel_->setText(badgeText);
    if (statusBadgeLabel_->property("statusState").toString() != key) {
        statusBadgeLabel_->setProperty("statusState", key);
        ThemeConfig::refreshWidgetStyle(statusBadgeLabel_);
    }
}

QString MachineStatusPanel::describeInstruction(Word ir) {
    if (ir == 0x0000) {
        return "No instruction";
    }
    if (!InstructionFactory::isValid(ir)) {
        return "Invalid opcode";
    }

    int opcode = (ir >> 12) & 0xF;
    int r = (ir >> 8) & 0xF;
    int s = (ir >> 4) & 0xF;
    int t = ir & 0xF;
    int xy = ir & 0xFF;

    auto regStr = [](int reg) {
        return QString("R%1").arg(reg, 1, 16).toUpper();
    };
    auto hexByte = [](int val) {
        return "0x" + QString("%1").arg(val, 2, 16, QChar('0')).toUpper();
    };

    switch (opcode) {
        case 0x1: return QString("LOAD %1 ← [%2]").arg(regStr(r), hexByte(xy));
        case 0x2: return QString("LOAD %1 ← %2").arg(regStr(r), hexByte(xy));
        case 0x3:
            if (xy == 0xFF) return QString("SCREEN ← %1").arg(regStr(r));
            return QString("STORE [%1] ← %2").arg(hexByte(xy), regStr(r));
        case 0x4: return QString("MOVE %1 ← %2").arg(regStr(t), regStr(s));
        case 0x5: return QString("ADD %1 ← %2+%3").arg(regStr(r), regStr(s), regStr(t));
        case 0x6: return QString("FADD %1 ← %2+%3").arg(regStr(r), regStr(s), regStr(t));
        case 0x7: return QString("OR %1 ← %2|%3").arg(regStr(r), regStr(s), regStr(t));
        case 0x8: return QString("AND %1 ← %2&%3").arg(regStr(r), regStr(s), regStr(t));
        case 0x9: return QString("XOR %1 ← %2^%3").arg(regStr(r), regStr(s), regStr(t));
        case 0xA: return QString("ROT %1 >> %2").arg(regStr(r)).arg(t);
        case 0xB: return QString("JMP %1 (%2=R0)").arg(hexByte(xy), regStr(r));
        case 0xC: return QString("HALT");
        default:  return "Unknown";
    }
}
