#pragma once

#include <QColor>
#include <QString>

namespace QuickUI::Components::Internal {

struct ThemeTokens
{
    QString id;
    QString name;

    QColor accent;
    QColor accentHover;
    QColor accentPressed;
    QColor accentDisabled;
    QColor iconColor;
    QColor iconColorPressed;
    QColor buttonHover;
    QColor buttonPressed;
    QColor trackBg;
    QColor trackBuffer;
    QColor handleBorder;
    QColor textPrimary;
    QColor textSecondary;
    QColor textDisabled;
    QColor textOnAccent;
    QColor surface;
    QColor surfaceHover;
    QColor separator;
    QColor inputBg;
    QColor inputBorder;
    QColor inputFocus;
    QColor inputText;
    QColor inputPlaceholder;

    int buttonSize = 34;
    int buttonRadius = 6;
    int inputHeight = 36;
    int inputRadius = 6;
    int trackHeight = 4;
    int handleSize = 14;

    QString fontFamily;
    int fontSize = 16;
    int fontSizeLabel = 13;
    int fontSizeCaption = 11;

    int durationFast = 80;
    int durationNormal = 120;
    bool reducedMotion = false;
};

} // namespace QuickUI::Components::Internal
