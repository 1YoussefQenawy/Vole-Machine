#include "HomePage.h"
#include "CellFlashAnimator.h"
#include "ThemeConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

struct TeamMember {
    const char* name;
    const char* id;
};

constexpr TeamMember kTeam[] = {
    {"Ibrahim Mohamed Hosny", "20250006"},
    {"Youssef Sayed Helmy",   "20251496"},
    {"Seif Khaled Ragab",     "20251205"},
    {"Youssef Ehab Ahmed",    "20242428"},
};

struct MetaSection {
    const char* title;
    const char* value;
};

constexpr MetaSection kMetaSections[] = {
    {"Section:",                  "20"},
    {"Technical Assistant (TA):", "Mahetab"},
    {"Under the supervision of:", "Dr. Mohamed El-Ramly"},
    {"Academic Year:",            "2026"},
};

// Applies CSS-equivalent filter: grayscale(amount), where amount is in [0.0, 1.0].
QPixmap applyGrayscaleFilter(const QPixmap& src, double amount) {
    if (src.isNull()) return src;
    QImage img = src.toImage().convertToFormat(QImage::Format_ARGB32);
    const double keep = 1.0 - amount;

    for (int y = 0; y < img.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const int r = qRed(line[x]);
            const int g = qGreen(line[x]);
            const int b = qBlue(line[x]);
            const int a = qAlpha(line[x]);
            // Standard CSS grayscale luminance (BT.709)
            const double lum = 0.2126 * r + 0.7152 * g + 0.0722 * b;
            const int nr = qBound(0, static_cast<int>(keep * r + amount * lum + 0.5), 255);
            const int ng = qBound(0, static_cast<int>(keep * g + amount * lum + 0.5), 255);
            const int nb = qBound(0, static_cast<int>(keep * b + amount * lum + 0.5), 255);
            line[x] = qRgba(nr, ng, nb, a);
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap loadHomeBackground() {
    const QStringList candidates = {
        QStringLiteral("C:/Users/slim/OneDrive/Desktop/vole-machine/volemachine-project/resources/home-background.jpeg"),
        QDir::currentPath() + QStringLiteral("/resources/home-background.jpeg"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/resources/home-background.jpeg"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../resources/home-background.jpeg"),
        QStringLiteral(":/home-background.jpeg")
    };

    for (const QString& path : candidates) {
        if (QFileInfo::exists(path)) {
            QPixmap pix(path);
            if (!pix.isNull()) {
                return applyGrayscaleFilter(pix, 0.60);
            }
        }
    }
    return QPixmap();
}

} // namespace

HomePage::HomePage(QWidget* parent)
    : QWidget(parent), bgPixmap_(loadHomeBackground()) {
    setObjectName("HomePage");

    // Left margin moved 30px back to the left (132 -> 102)
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(102, 24, 42, 28);
    root->setSpacing(38);

    // ------------------------------------------------------------------
    // Left section:
    //   1. VOLE (white) MACHINE (gold)
    //   2. Subtitle: Explore. Execute. Learn.
    //   3. Gold horizontal accent bar
    //   4. 4 Team members: Name | vertical bar | Gold ID
    //   5. 4-Section academic metadata row (exact same width as student rows,
    //      values have exact same font size as names in bold, unclipped)
    //   6. Gold pill-rounded "▶  START PROGRAM" button directly beneath
    // ------------------------------------------------------------------
    constexpr int kLeftBlockWidth = 556;
    auto* left = new QWidget(this);
    left->setObjectName("HomeLeftSection");
    left->setFixedWidth(kLeftBlockWidth);
    auto* leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // Preserved height from compressing title/slogan and student rows is added to upper spacing
    leftLayout->addStretch(18);

    titleLabel_ = new QLabel(
        QStringLiteral("<span style=\"color:#FFFFFF;\">VOLE</span> "
                       "<span style=\"color:#DBA858;\">MACHINE</span>"),
        left);
    titleLabel_->setObjectName("HomeTitle");
    titleLabel_->setTextFormat(Qt::RichText);
    leftLayout->addWidget(titleLabel_);

    leftLayout->addSpacing(0);

    auto* subtitleLabel = new QLabel(QStringLiteral("Explore. Execute. Learn."), left);
    subtitleLabel->setObjectName("HomeSubtitle");
    leftLayout->addWidget(subtitleLabel);

    leftLayout->addSpacing(15);

    auto* accentLine = new QFrame(left);
    accentLine->setObjectName("HomeGoldAccentLine");
    accentLine->setFixedSize(145, 3);
    leftLayout->addWidget(accentLine);

    leftLayout->addSpacing(16);

    auto* teamWidget = new QWidget(left);
    teamWidget->setFixedWidth(kLeftBlockWidth);
    auto* teamGrid = new QGridLayout(teamWidget);
    teamGrid->setContentsMargins(0, 0, 0, 0);
    teamGrid->setHorizontalSpacing(22);
    teamGrid->setVerticalSpacing(6);

    int row = 0;
    for (const TeamMember& member : kTeam) {
        auto* nameLabel = new QLabel(QString::fromUtf8(member.name), teamWidget);
        nameLabel->setObjectName("HomeMemberName");

        auto* sep = new QFrame(teamWidget);
        sep->setObjectName("HomeMemberSeparator");
        sep->setFixedWidth(1);
        sep->setFixedHeight(18);

        auto* idLabel = new QLabel(QString::fromUtf8(member.id), teamWidget);
        idLabel->setObjectName("HomeMemberId");
        idLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        teamGrid->addWidget(nameLabel, row, 0, Qt::AlignLeft | Qt::AlignVCenter);
        teamGrid->addWidget(sep, row, 1, Qt::AlignCenter);
        teamGrid->addWidget(idLabel, row, 2, Qt::AlignRight | Qt::AlignVCenter);
        ++row;
    }
    teamGrid->setColumnStretch(0, 1);
    leftLayout->addWidget(teamWidget);

    leftLayout->addSpacing(26);

    // 4-section metadata row: exact same width (kLeftBlockWidth) as the student rows above
    auto* metaRowWidget = new QWidget(left);
    metaRowWidget->setObjectName("HomeMetaRow");
    metaRowWidget->setFixedWidth(kLeftBlockWidth);
    auto* metaRowLayout = new QHBoxLayout(metaRowWidget);
    metaRowLayout->setContentsMargins(0, 0, 0, 0);
    metaRowLayout->setSpacing(0);

    constexpr int metaCount = static_cast<int>(sizeof(kMetaSections) / sizeof(kMetaSections[0]));
    for (int i = 0; i < metaCount; ++i) {
        auto* secWidget = new QWidget(metaRowWidget);
        secWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        auto* secLayout = new QVBoxLayout(secWidget);
        secLayout->setContentsMargins(2, 0, 2, 0);
        secLayout->setSpacing(5);

        auto* secTitle = new QLabel(QString::fromUtf8(kMetaSections[i].title), secWidget);
        secTitle->setObjectName("HomeMetaTitle");
        secTitle->setAlignment(Qt::AlignCenter);

        auto* secValue = new QLabel(QString::fromUtf8(kMetaSections[i].value), secWidget);
        secValue->setObjectName("HomeMetaValue");
        secValue->setAlignment(Qt::AlignCenter);

        secLayout->addWidget(secTitle);
        secLayout->addWidget(secValue);
        metaRowLayout->addWidget(secWidget);

        if (i < metaCount - 1) {
            metaRowLayout->addStretch(1);
            auto* goldSep = new QFrame(metaRowWidget);
            goldSep->setObjectName("HomeMetaGoldSeparator");
            goldSep->setFixedWidth(1);
            goldSep->setFixedHeight(42);
            metaRowLayout->addWidget(goldSep, 0, Qt::AlignVCenter);
            metaRowLayout->addStretch(1);
        }
    }
    leftLayout->addWidget(metaRowWidget);

    // Button placed right under the 4-sections data row (~70% width, centered in left block)
    leftLayout->addSpacing(26);

    auto* buttonRow = new QHBoxLayout();
    buttonRow->setContentsMargins(0, 0, 0, 0);
    startButton_ = new QPushButton(QStringLiteral("  START PROGRAM"), left);
    startButton_->setObjectName("HomeStartButton");
    startButton_->setCursor(Qt::PointingHandCursor);
    startButton_->setFixedSize(350, 50);
    {
        QPixmap playPix(18, 18);
        playPix.fill(Qt::transparent);
        QPainter p(&playPix);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(14, 21, 32));
        QPolygonF tri;
        tri << QPointF(3.0, 2.0) << QPointF(16.0, 9.0) << QPointF(3.0, 16.0);
        p.drawPolygon(tri);
        startButton_->setIcon(QIcon(playPix));
        startButton_->setIconSize(QSize(15, 15));
    }
    buttonRow->addStretch(1);
    buttonRow->addWidget(startButton_);
    buttonRow->addStretch(1);
    leftLayout->addLayout(buttonRow);

    // Larger bottom stretch keeps the entire left section raised ~50px higher
    leftLayout->addStretch(26);

    // ------------------------------------------------------------------
    // Right section: 8 x 8 decorative memory grid (64 cells: 00..3F)
    // inside a subtle translucent outer frame matching the reference image
    // ------------------------------------------------------------------
    auto* rightOuter = new QWidget(this);
    rightOuter->setObjectName("HomeRightOuter");
    auto* rightOuterLayout = new QVBoxLayout(rightOuter);
    rightOuterLayout->setContentsMargins(0, 0, 0, 0);
    rightOuterLayout->setSpacing(0);

    rightOuterLayout->addStretch(10);

    auto* matrixFrame = new QFrame(rightOuter);
    matrixFrame->setObjectName("HomeMatrixFrame");
    matrixFrame->setMinimumSize(570, 545);
    matrixFrame->setMaximumSize(630, 605);
    matrixFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightGridContainer_ = matrixFrame;

    auto* grid = new QGridLayout(matrixFrame);
    grid->setContentsMargins(18, 18, 18, 18);
    grid->setSpacing(ThemeConfig::HOME_CELL_SPACING);

    const int rows = ThemeConfig::HOME_MATRIX_ROWS;
    const int cols = ThemeConfig::HOME_MATRIX_COLS;
    cells_.reserve(rows * cols);

    for (int r = 0; r < rows; ++r) {
        grid->setRowStretch(r, 1);
    }
    for (int c = 0; c < cols; ++c) {
        grid->setColumnStretch(c, 1);
    }

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            auto* cell = new QFrame(matrixFrame);
            cell->setObjectName("HomeMatrixCell");
            cell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            auto* cellLayout = new QVBoxLayout(cell);
            cellLayout->setContentsMargins(0, 0, 0, 0);
            cellLayout->setSpacing(0);

            const int index = r * cols + c;
            auto* label = new QLabel(QString("%1").arg(index, 2, 16, QChar('0')).toUpper(), cell);
            label->setObjectName("HomeMatrixCellLabel");
            label->setAlignment(Qt::AlignCenter);
            cellLayout->addWidget(label);

            grid->addWidget(cell, r, c);
            cells_.append(cell);
        }
    }

    auto* matrixRow = new QHBoxLayout();
    matrixRow->setContentsMargins(0, 0, 0, 0);
    matrixRow->addStretch(2);
    matrixRow->addWidget(matrixFrame, 10);
    matrixRow->addStretch(1);

    rightOuterLayout->addLayout(matrixRow, 80);
    rightOuterLayout->addStretch(10);

    root->addWidget(left, 0);
    root->addWidget(rightOuter, 1);

    animator_ = new CellFlashAnimator(this);
    animator_->setCells(cells_);

    connect(startButton_, &QPushButton::clicked, this, &HomePage::startRequested);
}

void HomePage::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillRect(rect(), QColor(8, 14, 24));
    if (!bgPixmap_.isNull()) {
        const QPixmap scaled = bgPixmap_.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(0, 0, scaled);
    }
}

void HomePage::startIntroAnimation() {
    animator_->start(ThemeConfig::HOME_INTRO_EVENTS,
                     ThemeConfig::INTRO_EVENT_INTERVAL_MS,
                     ThemeConfig::INTRO_HIGHLIGHT_MS);
}

void HomePage::stopIntroAnimation() {
    animator_->stop();
}
