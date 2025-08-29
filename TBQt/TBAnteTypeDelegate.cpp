#include "TBAnteTypeDelegate.hpp"

#include <QComboBox>

TBAnteTypeDelegate::TBAnteTypeDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* TBAnteTypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    
    QComboBox* comboBox = new QComboBox(parent);
    comboBox->addItem(tr("None"), 0);
    comboBox->addItem(tr("Traditional"), 1);
    comboBox->addItem(tr("Big Blind"), 2);
    return comboBox;
}

void TBAnteTypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;
    
    int anteType = index.model()->data(index, Qt::EditRole).toInt();
    int comboIndex = comboBox->findData(anteType);
    if (comboIndex >= 0)
    {
        comboBox->setCurrentIndex(comboIndex);
    }
}

void TBAnteTypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;
    
    int anteType = comboBox->currentData().toInt();
    model->setData(index, anteType, Qt::EditRole);
}

QString TBAnteTypeDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    Q_UNUSED(locale)
    
    int anteType = value.toInt();
    return anteTypeToString(anteType);
}

QString TBAnteTypeDelegate::anteTypeToString(int anteType)
{
    switch (anteType)
    {
        case 0: return QObject::tr("None");
        case 1: return QObject::tr("Traditional");
        case 2: return QObject::tr("Big Blind");
        default: return QObject::tr("Unknown");
    }
}

int TBAnteTypeDelegate::anteTypeFromString(const QString& text)
{
    if (text == QObject::tr("None")) return 0;
    if (text == QObject::tr("Traditional")) return 1;
    if (text == QObject::tr("Big Blind")) return 2;
    return 0; // Default to None
}