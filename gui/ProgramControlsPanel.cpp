#include "ProgramControlsPanel.h"
#include "ThemeConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QPolygonF>

ProgramControlsPanel::ProgramControlsPanel(QWidget* parent)
    : QFrame(parent) {
    setObjectName("ProgramControlsPanel");
    setProperty("panelRole", "card");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(ThemeConfig::PANEL_PADDING, ThemeConfig::PANEL_PADDING,
                                   ThemeConfig::PANEL_PADDING, ThemeConfig::PANEL_PADDING);
    mainLayout->setSpacing(ThemeConfig::PANEL_SPACING);

    // --- Header ---
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(6);
    auto* titleLabel = new QLabel("PROGRAM", this);
    titleLabel->setObjectName("SectionTitle");
    sourceModeBadge_ = new QLabel("MANUAL", this);
    sourceModeBadge_->setObjectName("SourceModeBadge");
    sourceModeBadge_->setProperty("mode", "manual");

    homeButton_ = new QPushButton(" Home", this);
    homeButton_->setObjectName("BackHomeButton");
    homeButton_->setCursor(Qt::PointingHandCursor);
    homeButton_->setToolTip("Return to Home Page");
    {
        QPixmap homePix(16, 16);
        homePix.fill(Qt::transparent);
        QPainter p(&homePix);
        p.setRenderHint(QPainter::Antialiasing, true);
        QPen pen(QColor(154, 174, 196), 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        // Roof
        QPolygonF roof;
        roof << QPointF(2.0, 7.5) << QPointF(8.0, 2.2) << QPointF(14.0, 7.5);
        p.drawPolyline(roof);
        // House body & door
        QPolygonF body;
        body << QPointF(4.0, 6.8) << QPointF(4.0, 13.5) << QPointF(6.8, 13.5)
             << QPointF(6.8, 9.8) << QPointF(9.2, 9.8) << QPointF(9.2, 13.5)
             << QPointF(12.0, 13.5) << QPointF(12.0, 6.8);
        p.drawPolyline(body);
        homeButton_->setIcon(QIcon(homePix));
        homeButton_->setIconSize(QSize(14, 14));
    }

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(sourceModeBadge_);
    headerLayout->addStretch();
    headerLayout->addWidget(homeButton_);
    mainLayout->addLayout(headerLayout);

    // --- File selection ---
    auto* fileLabel = new QLabel("Program file", this);
    fileLabel->setObjectName("FieldLabel");
    mainLayout->addWidget(fileLabel);

    auto* fileRowLayout = new QHBoxLayout();
    fileRowLayout->setSpacing(6);
    filePathEdit_ = new QLineEdit(this);
    filePathEdit_->setObjectName("FilePathEdit");
    filePathEdit_->setPlaceholderText("No file selected");

    browseButton_ = new QPushButton("Browse", this);
    browseButton_->setObjectName("SecondaryButton");
    browseButton_->setCursor(Qt::PointingHandCursor);
    browseButton_->setToolTip("Browse for a Vole program file");

    clearSourceButton_ = new QPushButton("Clear", this);
    clearSourceButton_->setObjectName("GhostButton");
    clearSourceButton_->setCursor(Qt::PointingHandCursor);
    clearSourceButton_->setToolTip("Clear file selection and editor");

    fileRowLayout->addWidget(filePathEdit_, 1);
    fileRowLayout->addWidget(browseButton_);
    fileRowLayout->addWidget(clearSourceButton_);
    mainLayout->addLayout(fileRowLayout);

    // --- Start address ---
    auto* addrRowLayout = new QHBoxLayout();
    addrRowLayout->setSpacing(8);
    auto* addrLabel = new QLabel("Start address (even, 00–FE)", this);
    addrLabel->setObjectName("FieldLabel");
    startAddressEdit_ = new QLineEdit("0x00", this);
    startAddressEdit_->setObjectName("AddressInput");
    startAddressEdit_->setAlignment(Qt::AlignCenter);
    startAddressEdit_->setMaximumWidth(80);
    startAddressEdit_->setToolTip("Even hexadecimal address from 0x00 to 0xFE");
    addrRowLayout->addWidget(addrLabel, 1);
    addrRowLayout->addWidget(startAddressEdit_);
    mainLayout->addLayout(addrRowLayout);

    // --- Instruction editor ---
    auto* editorLabel = new QLabel("Instructions (hex, ';' comments)", this);
    editorLabel->setObjectName("FieldLabel");
    mainLayout->addWidget(editorLabel);

    programEditor_ = new QPlainTextEdit(this);
    programEditor_->setObjectName("ProgramEditor");
    programEditor_->setPlaceholderText(
        "2001  ; R0 = 01\n"
        "2102  ; R1 = 02\n"
        "5201  ; R2 = R0 + R1\n"
        "32FF  ; screen <- R2\n"
        "C000  ; halt");
    programEditor_->setMinimumHeight(120);
    mainLayout->addWidget(programEditor_, 1);

    loadProgramButton_ = new QPushButton("Load Program", this);
    loadProgramButton_->setObjectName("PrimaryActionButton");
    loadProgramButton_->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(loadProgramButton_);

    auto* divider = new QFrame(this);
    divider->setObjectName("PanelDivider");
    divider->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(divider);

    // --- Execution controls ---
    auto* execTitle = new QLabel("EXECUTION", this);
    execTitle->setObjectName("SectionTitle");
    mainLayout->addWidget(execTitle);

    auto* execGridLayout = new QGridLayout();
    execGridLayout->setHorizontalSpacing(8);
    execGridLayout->setVerticalSpacing(8);

    runAllButton_ = new QPushButton("Run All", this);
    runAllButton_->setObjectName("RunAllButton");
    runAllButton_->setCursor(Qt::PointingHandCursor);
    runAllButton_->setToolTip("Execute until halt (C000) or error");

    stepButton_ = new QPushButton("Step", this);
    stepButton_->setObjectName("StepButton");
    stepButton_->setCursor(Qt::PointingHandCursor);
    stepButton_->setToolTip("Execute exactly one instruction");

    haltButton_ = new QPushButton("Halt", this);
    haltButton_->setObjectName("HaltButton");
    haltButton_->setCursor(Qt::PointingHandCursor);
    haltButton_->setToolTip("Halt the CPU immediately (also stops Run All)");

    resetButton_ = new QPushButton("Reset Machine", this);
    resetButton_->setObjectName("ResetButton");
    resetButton_->setCursor(Qt::PointingHandCursor);
    resetButton_->setToolTip("Reset CPU, registers, memory and screen");

    execGridLayout->addWidget(runAllButton_, 0, 0);
    execGridLayout->addWidget(stepButton_, 0, 1);
    execGridLayout->addWidget(haltButton_, 1, 0, 1, 2);
    execGridLayout->addWidget(resetButton_, 2, 0, 1, 2);
    mainLayout->addLayout(execGridLayout);

    // --- Signals ---
    connect(browseButton_, &QPushButton::clicked, this, &ProgramControlsPanel::onBrowseClicked);
    connect(clearSourceButton_, &QPushButton::clicked, this, &ProgramControlsPanel::onClearSourceClicked);
    connect(programEditor_, &QPlainTextEdit::textChanged, this, &ProgramControlsPanel::onEditorTextChanged);
    connect(filePathEdit_, &QLineEdit::textEdited, this, &ProgramControlsPanel::onFilePathEdited);

    connect(homeButton_, &QPushButton::clicked, this, &ProgramControlsPanel::homeRequested);
    connect(loadProgramButton_, &QPushButton::clicked, this, &ProgramControlsPanel::loadProgramRequested);
    connect(stepButton_, &QPushButton::clicked, this, &ProgramControlsPanel::stepRequested);
    connect(runAllButton_, &QPushButton::clicked, this, &ProgramControlsPanel::runAllRequested);
    connect(haltButton_, &QPushButton::clicked, this, &ProgramControlsPanel::haltRequested);
    connect(resetButton_, &QPushButton::clicked, this, &ProgramControlsPanel::resetRequested);
}

QString ProgramControlsPanel::selectedFilePath() const {
    return filePathEdit_->text().trimmed();
}

QString ProgramControlsPanel::editorText() const {
    return programEditor_->toPlainText();
}

QString ProgramControlsPanel::startAddressText() const {
    return startAddressEdit_->text().trimmed();
}

bool ProgramControlsPanel::isUsingFileSource() const {
    return usingFileSource_ && !selectedFilePath().isEmpty();
}

void ProgramControlsPanel::setExecutionControls(bool running, bool machineHalted) {
    const bool idle = !running;
    filePathEdit_->setEnabled(idle);
    browseButton_->setEnabled(idle);
    clearSourceButton_->setEnabled(idle);
    startAddressEdit_->setEnabled(idle);
    programEditor_->setEnabled(idle);
    loadProgramButton_->setEnabled(idle);
    runAllButton_->setEnabled(idle && !machineHalted);
    stepButton_->setEnabled(idle && !machineHalted);
    resetButton_->setEnabled(idle);
    haltButton_->setEnabled(running || !machineHalted);
}

void ProgramControlsPanel::setProgramText(const QString& text, const QString& filePath) {
    suppressEditorSignal_ = true;
    programEditor_->setPlainText(text);
    filePathEdit_->setText(filePath);
    usingFileSource_ = !filePath.isEmpty();
    suppressEditorSignal_ = false;
    updateSourceBadge();
}

void ProgramControlsPanel::setStartAddressText(const QString& text) {
    startAddressEdit_->setText(text);
}

void ProgramControlsPanel::onBrowseClicked() {
    QString initialDir = QDir::currentPath();
    if (!filePathEdit_->text().trimmed().isEmpty()) {
        QFileInfo fi(filePathEdit_->text().trimmed());
        if (fi.absoluteDir().exists()) {
            initialDir = fi.absolutePath();
        }
    }

    const QString chosenFile = QFileDialog::getOpenFileName(
        this, "Select Vole Program File", initialDir,
        "Program Files (*.txt *.vole *.hex);;All Files (*.*)");

    if (chosenFile.isEmpty()) {
        return;
    }

    QFile file(chosenFile);
    QString content;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        content = in.readAll();
        file.close();
    }

    setProgramText(content, chosenFile);
}

void ProgramControlsPanel::onClearSourceClicked() {
    suppressEditorSignal_ = true;
    filePathEdit_->clear();
    programEditor_->clear();
    usingFileSource_ = false;
    suppressEditorSignal_ = false;
    updateSourceBadge();
}

void ProgramControlsPanel::onEditorTextChanged() {
    if (suppressEditorSignal_) return;
    usingFileSource_ = false;   // user edited the instructions manually
    updateSourceBadge();
}

void ProgramControlsPanel::onFilePathEdited(const QString& path) {
    usingFileSource_ = !path.trimmed().isEmpty();
    if (usingFileSource_) {
        QFile file(path.trimmed());
        if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            suppressEditorSignal_ = true;
            QTextStream in(&file);
            programEditor_->setPlainText(in.readAll());
            suppressEditorSignal_ = false;
        }
    }
    updateSourceBadge();
}

void ProgramControlsPanel::updateSourceBadge() {
    if (isUsingFileSource()) {
        sourceModeBadge_->setText("FILE");
        sourceModeBadge_->setProperty("mode", "file");
    } else {
        sourceModeBadge_->setText("MANUAL");
        sourceModeBadge_->setProperty("mode", "manual");
    }
    ThemeConfig::refreshWidgetStyle(sourceModeBadge_);
}
