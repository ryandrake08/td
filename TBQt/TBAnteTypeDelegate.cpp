#include "TBAnteTypeDelegate.hpp"
#include "TournamentSession.hpp"

#include <QComboBox>

TBAnteTypeDelegate::TBAnteTypeDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

QWidget* TBAnteTypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)

    // Check if there's an ante for this row
    QModelIndex anteIndex = index.model()->index(index.row(), 5); // Ante column
    int ante = anteIndex.data(Qt::EditRole).toInt();
    if(ante <= 0)
    {
        return nullptr; // No editor when no ante
    }

    QComboBox* comboBox = new QComboBox(parent);
    comboBox->addItem(TournamentSession::toString(TournamentSession::AnteType::Traditional), TournamentSession::toInt(TournamentSession::AnteType::Traditional));
    comboBox->addItem(TournamentSession::toString(TournamentSession::AnteType::BigBlind), TournamentSession::toInt(TournamentSession::AnteType::BigBlind));
    return comboBox;
}

void TBAnteTypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox)
        return;

    int anteType = index.model()->data(index, Qt::EditRole).toInt();
    int comboIndex = comboBox->findData(anteType);
    if(comboIndex >= 0)
    {
        comboBox->setCurrentIndex(comboIndex);
    }
}

void TBAnteTypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox)
        return;

    int anteType = comboBox->currentData().toInt();
    model->setData(index, anteType, Qt::EditRole);
}

QString TBAnteTypeDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    Q_UNUSED(locale)

    // The model already handles the display logic in its data() method
    // Just return the formatted text that the model provides
    return value.toString();
}