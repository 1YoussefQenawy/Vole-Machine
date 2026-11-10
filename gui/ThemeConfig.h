#ifndef THEME_CONFIG_H
#define THEME_CONFIG_H

#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QWidget>
#include <QStyle>
#include <QCoreApplication>
#include <QDir>
#include <algorithm>
#include <vector>
#include <utility>

namespace ThemeConfig {

// ============================================================================
// Window
// ============================================================================
constexpr int WINDOW_DEFAULT_WIDTH  = 1440;
constexpr int WINDOW_DEFAULT_HEIGHT = 880;
constexpr int WINDOW_MIN_WIDTH      = 1180;
constexpr int WINDOW_MIN_HEIGHT     = 740;

// ============================================================================
// Workspace layout
// ============================================================================
constexpr int WORKSPACE_MARGIN      = 12;
constexpr int WORKSPACE_SPACING     = 10;

constexpr int PANEL_PADDING         = 10;
constexpr int PANEL_PADDING_COMPACT = 8;
constexpr int PANEL_SPACING         = 6;

constexpr int SIDEBAR_MIN_WIDTH     = 296;
constexpr int SIDEBAR_MAX_WIDTH     = 332;

// Top row (Screen+CPU card | Registers card) vs. Memory matrix stretch ratios
// Reduced top row height (~22%) transfers substantial vertical room to Memory (~78%)
constexpr int TOP_ROW_STRETCH       = 22;
constexpr int MEMORY_ROW_STRETCH    = 78;
constexpr int TOP_ROW_MAX_HEIGHT    = 206;

// Integrated CPU sub-card inside ScreenPanel (~0.5x previous status column width)
constexpr int CPU_SUBCARD_WIDTH     = 96;

// Registers panel receives the horizontal width recovered from CPU info
constexpr int REGISTER_PANEL_WIDTH  = 340;
constexpr int REGISTER_GRID_ROWS    = 4;
constexpr int REGISTER_GRID_COLS    = 4;
constexpr int REGISTER_GRID_SPACING = 5;

constexpr int MEMORY_MATRIX_ROWS    = 16;
constexpr int MEMORY_MATRIX_COLS    = 16;
constexpr int MEMORY_MATRIX_SPACING = 3;

// Simulated screen internal heights:
// Output screen reduced to ~0.4x its previous ~120px height (~46px),
// Execution log height preserved (~82px).
constexpr int SCREEN_OUTPUT_HEIGHT  = 46;
constexpr int SCREEN_LOG_HEIGHT     = 82;
constexpr int SCREEN_FOOTER_MAX_BYTES = 12;

// Home page: 8x8 decorative memory grid (64 cells)
constexpr int HOME_MARGIN_H         = 68;
constexpr int HOME_MARGIN_V         = 40;
constexpr int HOME_SECTION_GAP      = 56;
constexpr int HOME_MATRIX_ROWS      = 8;
constexpr int HOME_MATRIX_COLS      = 8;
constexpr int HOME_CELL_SPACING     = 10;

// ============================================================================
// Animation timings (continuous looping cycles)
// ============================================================================
constexpr int HOME_INTRO_EVENTS       = 3;
constexpr int MEMORY_INTRO_EVENTS     = 4;
constexpr int INTRO_EVENT_INTERVAL_MS = 100;
constexpr int INTRO_HIGHLIGHT_MS      = 200;
constexpr int HOME_INTRO_DELAY_MS     = 80;
constexpr int PAGE_FADE_MS            = 160;

// Accent colour keys used by the decorative flash animation.
// Each key maps to a [flash="key"] rule in theme.qss.
inline const QStringList FLASH_COLOR_KEYS = {
    QStringLiteral("purple"),
    QStringLiteral("orange"),
    QStringLiteral("green"),
    QStringLiteral("blue")
};

// ============================================================================
// Stylesheet Loader with @token Design Variable Resolution
// ============================================================================
inline QString loadStyleSheet() {
    QStringList candidatePaths = {
        QDir::currentPath() + "/resources/styles/theme.qss",
        QCoreApplication::applicationDirPath() + "/resources/styles/theme.qss",
        QCoreApplication::applicationDirPath() + "/../resources/styles/theme.qss",
        ":/styles/theme.qss"
    };

    QString rawQss;
    for (const QString& path : candidatePaths) {
        QFile file(path);
        if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            rawQss = in.readAll();
            file.close();
            if (!rawQss.isEmpty()) {
                break;
            }
        }
    }

    if (rawQss.isEmpty()) {
        return QString();
    }

    std::vector<std::pair<QString, QString>> variables;
    QRegularExpression varRegex(R"(^\s*(@[a-zA-Z0-9_]+)\s*:\s*([^;]+);\s*$)",
                                QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = varRegex.globalMatch(rawQss);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        variables.emplace_back(match.captured(1).trimmed(), match.captured(2).trimmed());
    }

    QString processed = rawQss;
    processed.remove(varRegex);

    std::sort(variables.begin(), variables.end(),
              [](const auto& a, const auto& b) { return a.first.length() > b.first.length(); });

    for (const auto& [key, val] : variables) {
        processed.replace(key, val);
    }

    return processed;
}

inline void refreshWidgetStyle(QWidget* widget) {
    if (!widget || !widget->style()) return;
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    const auto children = widget->findChildren<QWidget*>();
    for (QWidget* child : children) {
        if (child->style()) {
            child->style()->unpolish(child);
            child->style()->polish(child);
        }
    }
    widget->update();
}

} // namespace ThemeConfig

#endif // THEME_CONFIG_H
