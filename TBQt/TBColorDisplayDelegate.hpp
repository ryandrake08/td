#pragma once

#include <QStyledItemDelegate>

class TBColorDisplayDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    QColor parseColor(const QString& colorString) const;
    QString generateRandomColor() const;

public:
    explicit TBColorDisplayDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    bool eventFilter(QObject* watched, QEvent* event) override;

private Q_SLOTS:
    void onColorButtonClicked();
};