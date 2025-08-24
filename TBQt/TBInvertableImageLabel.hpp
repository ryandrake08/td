#pragma once

#include <QLabel>
#include <QPixmap>

class TBInvertableImageLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(bool imageInverted READ imageInverted WRITE setImageInverted)

    void updateImagePixmap();

public:
    explicit TBInvertableImageLabel(QWidget* parent = nullptr);

    void setOriginalPixmap(const QPixmap& pixmap);
    bool imageInverted() const;

public Q_SLOTS:
    void setImageInverted(bool inverted);

private:
    QPixmap m_originalPixmap;
    QPixmap m_invertedPixmap;
    bool m_imageInverted;
};