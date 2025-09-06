#include "TBDateEditDelegate.hpp"

#include <QDateEdit>
#include <QDateTime>

TBDateEditDelegate::TBDateEditDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

QWidget* TBDateEditDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto* editor = new QDateEdit(parent);
    editor->setDisplayFormat("yyyy-MM-dd");
    editor->setCalendarPopup(true);
    return editor;
}

void TBDateEditDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* dateEdit = qobject_cast<QDateEdit*>(editor);
    if(!dateEdit)
        return;

    QString dateString = index.model()->data(index, Qt::EditRole).toString();
    QDateTime dateTime = QDateTime::fromString(dateString, Qt::ISODate);

    if(dateTime.isValid())
    {
        dateEdit->setDate(dateTime.date());
    }
    else
    {
        // Default to current date if invalid
        dateEdit->setDate(QDate::currentDate());
    }
}

void TBDateEditDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* dateEdit = qobject_cast<QDateEdit*>(editor);
    if(!dateEdit)
        return;

    QDate date = dateEdit->date();
    QDateTime dateTime(date, QTime(0, 0, 0));
    QString isoString = dateTime.toString(Qt::ISODate);

    model->setData(index, isoString, Qt::EditRole);
}

void TBDateEditDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

QString TBDateEditDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    Q_UNUSED(locale)

    QString dateString = value.toString();
    QDateTime dateTime = QDateTime::fromString(dateString, Qt::ISODate);

    if(dateTime.isValid())
    {
        return dateTime.date().toString("MMM d, yyyy");
    }

    return dateString;
}