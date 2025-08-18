#pragma once

#include <QStyledItemDelegate>
#include <QModelIndex>

class QWidget;
class QStyleOptionViewItem;
class QPainter;
class QMouseEvent;

class TBManageButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

Q_SIGNALS:
    void buttonClicked(const QModelIndex& index);

public:
    explicit TBManageButtonDelegate(QObject* parent = nullptr);

    // QStyledItemDelegate interface
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};