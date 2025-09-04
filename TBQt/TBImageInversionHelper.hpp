#pragma once

#include <QIcon>
#include <QPixmap>

// Utility class for image inversion operations
// Provides shared functionality for invertible UI components
class TBImageInversionHelper
{
public:
    // Create an inverted copy of a pixmap (RGB inverted, alpha preserved)
    static QPixmap createInvertedPixmap(const QPixmap& original);

    // Create an inverted copy of an icon (all sizes inverted)
    static QIcon createInvertedIcon(const QIcon& original);
};