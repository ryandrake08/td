#pragma once

#include <QPushButton>
#include <QPixmap>

class TBInvertableButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool imageInverted READ imageInverted WRITE setImageInverted)

    void updateButtonIcon();

public:
    explicit TBInvertableButton(QWidget* parent = nullptr);
    explicit TBInvertableButton(const QString& text, QWidget* parent = nullptr);

    void setOriginalIcon(const QIcon& icon);
    bool imageInverted() const;

public Q_SLOTS:
    void setImageInverted(bool inverted);

private:
    QIcon m_originalIcon;
    QIcon m_invertedIcon;
    bool m_imageInverted;
};