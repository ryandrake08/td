#pragma once

#include <QStyledItemDelegate>

class TBBlindLevelDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TBBlindLevelDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    QString displayText(const QVariant& value, const QLocale& locale) const override;

    void setBlindLevels(const QVariantList& rounds);

private:
    QVariantList m_rounds;
};