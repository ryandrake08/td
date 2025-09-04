#include "TBImageInversionHelper.hpp"

#include <QImage>
#include <QPixmap>
#include <QSize>

QPixmap TBImageInversionHelper::createInvertedPixmap(const QPixmap& original)
{
    if(original.isNull())
    {
        return QPixmap();
    }

    QImage image = original.toImage().convertToFormat(QImage::Format_ARGB32);

    // Invert RGB values while preserving alpha
    for(int y = 0; y < image.height(); ++y)
    {
        QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for(int x = 0; x < image.width(); ++x)
        {
            QRgb pixel = line[x];
            int alpha = qAlpha(pixel);
            int red = 255 - qRed(pixel);
            int green = 255 - qGreen(pixel);
            int blue = 255 - qBlue(pixel);
            line[x] = qRgba(red, green, blue, alpha);
        }
    }

    return QPixmap::fromImage(image);
}

QIcon TBImageInversionHelper::createInvertedIcon(const QIcon& original)
{
    if(original.isNull())
    {
        return QIcon();
    }

    QIcon invertedIcon;
    QList<QSize> availableSizes = original.availableSizes();

    for(const QSize& size : availableSizes)
    {
        QPixmap originalPixmap = original.pixmap(size);
        QPixmap invertedPixmap = createInvertedPixmap(originalPixmap);
        invertedIcon.addPixmap(invertedPixmap);
    }

    return invertedIcon;
}