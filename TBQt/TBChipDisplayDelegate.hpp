#pragma once

#include <QColor>
#include <QMap>
#include <QStyledItemDelegate>

// Custom delegate for rendering chip colors as ellipses in table views
class TBChipDisplayDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TBChipDisplayDelegate(QObject* parent = nullptr);
    virtual ~TBChipDisplayDelegate() override;

    // QStyledItemDelegate interface
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    // Convert CSS color name or hex string to QColor
    QColor parseColor(const QString& colorString) const;

    // CSS color name lookup table (subset of most common colors)
    static QMap<QString, QString> createCssColorMap();
    static const QMap<QString, QString> cssColors;
};