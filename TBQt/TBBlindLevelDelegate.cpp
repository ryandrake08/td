#include "TBBlindLevelDelegate.hpp"

#include <QComboBox>

TBBlindLevelDelegate::TBBlindLevelDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

QWidget* TBBlindLevelDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QComboBox* comboBox = new QComboBox(parent);

    // Add "Start" for round 0 (before tournament starts)
    comboBox->addItem(tr("Start"), 0);

    // Add blind levels from rounds (skip round 0, start from round 1)
    for(int i = 1; i < m_rounds.size(); i++)
    {
        QString levelName = QString("Level %1").arg(i);
        comboBox->addItem(levelName, i);
    }

    // Add "Never" at the end (special value -1 indicates missing key)
    comboBox->addItem(tr("Never"), -1);

    return comboBox;
}

void TBBlindLevelDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox)
        return;

    QVariant data = index.model()->data(index, Qt::EditRole);
    if(!data.isValid() || data.isNull())
    {
        // Key doesn't exist, select "Never"
        int comboIndex = comboBox->findData(-1);
        if(comboIndex >= 0)
            comboBox->setCurrentIndex(comboIndex);
        return;
    }

    int blindLevel = data.toInt();
    int comboIndex = comboBox->findData(blindLevel);
    if(comboIndex >= 0)
    {
        comboBox->setCurrentIndex(comboIndex);
    }
}

void TBBlindLevelDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox)
        return;

    int blindLevel = comboBox->currentData().toInt();
    if(blindLevel < 0)
    {
        // "Never" selected - remove the key by setting to invalid QVariant
        model->setData(index, QVariant(), Qt::EditRole);
    }
    else
    {
        model->setData(index, blindLevel, Qt::EditRole);
    }
}

QString TBBlindLevelDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    Q_UNUSED(locale)

    // Check if the key exists in the data - if not, show "Never"
    if(!value.isValid() || value.isNull())
    {
        return tr("Never");
    }

    int blindLevel = value.toInt();

    if(blindLevel < 0)
    {
        return tr("Never");
    }

    if(blindLevel == 0)
    {
        return tr("Start");
    }

    return QString("Level %1").arg(blindLevel);
}

void TBBlindLevelDelegate::setBlindLevels(const QVariantList& rounds)
{
    m_rounds = rounds;
}