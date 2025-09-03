#pragma once

#include <QLabel>

class TBScalableLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int minimumFontSize READ minimumFontSize WRITE setMinimumFontSize)
    Q_PROPERTY(int maximumFontSize READ maximumFontSize WRITE setMaximumFontSize)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily)

public:
    explicit TBScalableLabel(QWidget* parent = nullptr);
    explicit TBScalableLabel(const QString& text, QWidget* parent = nullptr);

    void setMinimumFontSize(int size);
    void setMaximumFontSize(int size);
    int minimumFontSize() const;
    int maximumFontSize() const;

    void setFontFamily(const QString& family);
    QString fontFamily() const;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void updateFontSize();
    int calculateOptimalFontSize(const QRect& rect) const;

    int m_minimumFontSize;
    int m_maximumFontSize;
    QString m_fontFamily;
};