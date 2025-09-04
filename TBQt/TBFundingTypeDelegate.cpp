#include "TBFundingTypeDelegate.hpp"
#include "TournamentSession.hpp"

#include <QComboBox>

TBFundingTypeDelegate::TBFundingTypeDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

QWidget* TBFundingTypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QComboBox* comboBox = new QComboBox(parent);
    comboBox->addItem(TournamentSession::toString(TournamentSession::FundingType::Buyin), TournamentSession::toInt(TournamentSession::FundingType::Buyin));
    comboBox->addItem(TournamentSession::toString(TournamentSession::FundingType::Rebuy), TournamentSession::toInt(TournamentSession::FundingType::Rebuy));
    comboBox->addItem(TournamentSession::toString(TournamentSession::FundingType::Addon), TournamentSession::toInt(TournamentSession::FundingType::Addon));
    return comboBox;
}

void TBFundingTypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox)
        return;

    int fundingType = index.model()->data(index, Qt::EditRole).toInt();
    int comboIndex = comboBox->findData(fundingType);
    if(comboIndex >= 0)
    {
        comboBox->setCurrentIndex(comboIndex);
    }
}

void TBFundingTypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox)
        return;

    int fundingType = comboBox->currentData().toInt();
    model->setData(index, fundingType, Qt::EditRole);
}

QString TBFundingTypeDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    Q_UNUSED(locale)

    int fundingType = value.toInt();
    return TournamentSession::toString(TournamentSession::toFundingType(fundingType));
}