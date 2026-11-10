#include "ScreenPanel.h"
#include "MachineStatusPanel.h"
#include "ThemeConfig.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QStringList>
#include <QVBoxLayout>
#include <cstdint>

ScreenPanel::ScreenPanel(QWidget* parent)
    : QFrame(parent) {
    setObjectName("ScreenPanel");
    setProperty("panelRole", "card");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(ThemeConfig::PANEL_PADDING_COMPACT, ThemeConfig::PANEL_PADDING_COMPACT,
                                   ThemeConfig::PANEL_PADDING_COMPACT, ThemeConfig::PANEL_PADDING_COMPACT);
    mainLayout->setSpacing(6);

    // Header
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(6);
    auto* titleLabel = new QLabel("SCREEN OUTPUT & CPU", this);
    titleLabel->setObjectName("SectionTitle");
    auto* hintLabel = new QLabel("3RFF", this);
    hintLabel->setObjectName("PanelMetaBadge");
    byteCountBadge_ = new QLabel("0 BYTES", this);
    byteCountBadge_->setObjectName("ScreenByteBadge");
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(hintLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(byteCountBadge_);
    mainLayout->addLayout(headerLayout);

    // Horizontal content row: [Simulated Screen Bezel] [Merged CPU Sub-Card]
    auto* contentRow = new QHBoxLayout();
    contentRow->setContentsMargins(0, 0, 0, 0);
    contentRow->setSpacing(8);

    // Bezel -> glass
    auto* bezel = new QFrame(this);
    bezel->setObjectName("ScreenBezel");
    auto* bezelLayout = new QVBoxLayout(bezel);
    bezelLayout->setContentsMargins(4, 4, 4, 4);
    bezelLayout->setSpacing(0);

    auto* glass = new QFrame(bezel);
    glass->setObjectName("ScreenGlass");
    auto* glassLayout = new QVBoxLayout(glass);
    glassLayout->setContentsMargins(0, 0, 0, 0);
    glassLayout->setSpacing(0);

    // 1) Actual machine output (compact ~0.4x height)
    outputView_ = new QPlainTextEdit(glass);
    outputView_->setObjectName("ScreenOutputView");
    outputView_->setReadOnly(true);
    outputView_->setFrameShape(QFrame::NoFrame);
    outputView_->setPlaceholderText("_ awaiting output");
    outputView_->setFixedHeight(ThemeConfig::SCREEN_OUTPUT_HEIGHT);
    glassLayout->addWidget(outputView_);

    // 2) Execution log (status history, append-only; unchanged height)
    auto* logHeader = new QFrame(glass);
    logHeader->setObjectName("ScreenLogHeader");
    auto* logHeaderLayout = new QHBoxLayout(logHeader);
    logHeaderLayout->setContentsMargins(8, 2, 8, 2);
    auto* logTitle = new QLabel("EXECUTION LOG", logHeader);
    logTitle->setObjectName("ScreenLogTitle");
    logHeaderLayout->addWidget(logTitle);
    logHeaderLayout->addStretch();
    glassLayout->addWidget(logHeader);

    logArea_ = new QScrollArea(glass);
    logArea_->setObjectName("ScreenLogArea");
    logArea_->setFrameShape(QFrame::NoFrame);
    logArea_->setWidgetResizable(true);
    logArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    logArea_->setFixedHeight(ThemeConfig::SCREEN_LOG_HEIGHT);

    logContainer_ = new QWidget(logArea_);
    logContainer_->setObjectName("ScreenLogContainer");
    logLayout_ = new QVBoxLayout(logContainer_);
    logLayout_->setContentsMargins(8, 3, 8, 3);
    logLayout_->setSpacing(1);
    logLayout_->addStretch(1);
    logArea_->setWidget(logContainer_);
    glassLayout->addWidget(logArea_, 1);

    // Keep the newest log entry in view
    connect(logArea_->verticalScrollBar(), &QScrollBar::rangeChanged, this,
            [this](int, int max) { logArea_->verticalScrollBar()->setValue(max); });

    // 3) HEX / DEC indicators, bottom-left of the screen
    auto* footer = new QFrame(glass);
    footer->setObjectName("ScreenFooter");
    auto* footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(8, 3, 8, 3);
    footerLayout->setSpacing(14);
    hexLabel_ = new QLabel(footer);
    hexLabel_->setObjectName("ScreenStreamText");
    decLabel_ = new QLabel(footer);
    decLabel_->setObjectName("ScreenStreamText");
    footerLayout->addWidget(hexLabel_);
    footerLayout->addWidget(decLabel_);
    footerLayout->addStretch(1);
    glassLayout->addWidget(footer);

    bezelLayout->addWidget(glass);
    contentRow->addWidget(bezel, 1);

    // Merged CPU card positioned beside the output screen inside the same parent card
    statusPanel_ = new MachineStatusPanel(this);
    contentRow->addWidget(statusPanel_);

    mainLayout->addLayout(contentRow, 1);

    updateScreen("");
}

void ScreenPanel::updateScreen(const std::string& screenContent) {
    if (initialized_ && screenContent == lastContent_) {
        return;   // no new output: keep what is displayed
    }
    initialized_ = true;
    lastContent_ = screenContent;

    const int count = static_cast<int>(screenContent.size());
    byteCountBadge_->setText(count == 1 ? "1 BYTE" : QString("%1 BYTES").arg(count));

    if (screenContent.empty()) {
        outputView_->clear();
        hexLabel_->setText("HEX BYTES: —");
        decLabel_->setText("DEC VALUES: —");
        hexLabel_->parentWidget()->setToolTip(QString());
        return;
    }

    QString displayString;
    QStringList hexList;
    QStringList decList;

    for (unsigned char ch : screenContent) {
        const uint8_t byteVal = static_cast<uint8_t>(ch);
        const QString hexToken = QString("%1").arg(byteVal, 2, 16, QChar('0')).toUpper();
        hexList.append("0x" + hexToken);
        decList.append(QString::number(byteVal));

        if ((byteVal >= 32 && byteVal <= 126) || byteVal == '\n' || byteVal == '\r' || byteVal == '\t') {
            displayString.append(QChar(byteVal));
        } else {
            // Non-printable byte (e.g. numeric results such as 0x03): show it explicitly
            displayString.append(QString("[0x%1 / %2] ").arg(hexToken).arg(byteVal));
        }
    }

    outputView_->setPlainText(displayString);
    outputView_->verticalScrollBar()->setValue(outputView_->verticalScrollBar()->maximum());

    const int maxShown = ThemeConfig::SCREEN_FOOTER_MAX_BYTES;
    const bool truncated = hexList.size() > maxShown;
    const QString prefix = truncated ? QStringLiteral("… ") : QString();
    hexLabel_->setText("HEX BYTES: " + prefix + hexList.mid(hexList.size() - std::min<int>(maxShown, static_cast<int>(hexList.size()))).join(' '));
    decLabel_->setText("DEC VALUES: " + prefix + decList.mid(decList.size() - std::min<int>(maxShown, static_cast<int>(decList.size()))).join(' '));
    hexLabel_->parentWidget()->setToolTip("HEX BYTES: " + hexList.join(' ') + "\nDEC VALUES: " + decList.join(' '));
}

void ScreenPanel::appendLog(const QString& level, const QString& message) {
    ++logCount_;
    lastLog_ = message;

    auto* entry = new QLabel(QString("%1  %2").arg(logCount_, 3, 10, QChar('0')).arg(message), logContainer_);
    entry->setObjectName("ScreenLogEntry");
    entry->setProperty("level", level);
    entry->setWordWrap(true);
    entry->setTextInteractionFlags(Qt::TextSelectableByMouse);
    // Insert above the trailing stretch so entries stack top-down
    logLayout_->insertWidget(logLayout_->count() - 1, entry);
}
