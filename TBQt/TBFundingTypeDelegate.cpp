#include "TBFundingTypeDelegate.hpp"

#include <QComboBox>

TBFundingTypeDelegate::TBFundingTypeDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* TBFundingTypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    
    QComboBox* comboBox = new QComboBox(parent);
    comboBox->addItem(tr("Buy-in"), 0);
    comboBox->addItem(tr("Rebuy"), 1);
    comboBox->addItem(tr("Add-on"), 2);
    return comboBox;
}

void TBFundingTypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;
    
    int fundingType = index.model()->data(index, Qt::EditRole).toInt();
    int comboIndex = comboBox->findData(fundingType);
    if (comboIndex >= 0)
    {
        comboBox->setCurrentIndex(comboIndex);
    }
}

void TBFundingTypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;
    
    int fundingType = comboBox->currentData().toInt();
    model->setData(index, fundingType, Qt::EditRole);
}

QString TBFundingTypeDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    Q_UNUSED(locale)
    
    int fundingType = value.toInt();
    return fundingTypeToString(fundingType);
}

QString TBFundingTypeDelegate::fundingTypeToString(int fundingType)
{
    switch (fundingType)
    {
        case 0: return QObject::tr("Buy-in");
        case 1: return QObject::tr("Rebuy");
        case 2: return QObject::tr("Add-on");
        default: return QObject::tr("Unknown");
    }
}