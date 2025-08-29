#pragma once

#include <QStyledItemDelegate>

class TBFundingTypeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TBFundingTypeDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    QString displayText(const QVariant& value, const QLocale& locale) const override;

private:
    static QString fundingTypeToString(int fundingType);
};