#pragma once

#include <QColor>

// Utility functions for color analysis
namespace TBColorUtils
{
    // Calculate if a color is considered "dark" using ITU-R BT.601 coefficients
    inline bool colorIsDark(const QColor& color)
    {
        return 0.299F * color.redF() + 0.587F * color.greenF() + 0.114F * color.blueF() < 0.5;
    }
}