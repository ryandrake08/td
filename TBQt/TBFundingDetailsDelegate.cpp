#include "TBFundingDetailsDelegate.hpp"
#include "TBCurrency.hpp"

#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

TBFundingDetailsDelegate::TBFundingDetailsDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

QWidget* TBFundingDetailsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)

    QPushButton* button = new QPushButton(tr("Details..."), parent);

    QObject::connect(button, &QPushButton::clicked, [button, index, this]() {
        QVariantMap rowData = index.model()->index(index.row(), 0).data(Qt::UserRole).toMap();
        if (showDetailsDialog(button, rowData))
        {
            button->setProperty("modifiedData", rowData);
        }
    });

    return button;
}

void TBFundingDetailsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
}

void TBFundingDetailsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QPushButton* button = qobject_cast<QPushButton*>(editor);
    if (!button)
        return;

    QVariantMap modifiedData = button->property("modifiedData").toMap();
    if (!modifiedData.isEmpty())
    {
        // Update the row data with modified funding data
        model->setData(model->index(index.row(), 0), modifiedData, Qt::UserRole);
        model->dataChanged(model->index(index.row(), 0), model->index(index.row(), model->columnCount()-1));
    }
}

QString TBFundingDetailsDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    Q_UNUSED(locale)

    // Display the cost amount
    return QString("$%1").arg(value.toDouble(), 0, 'f', 2);
}

bool TBFundingDetailsDelegate::showDetailsDialog(QWidget* parent, QVariantMap& fundingData) const
{
    QDialog dialog(parent);
    dialog.setWindowTitle(tr("Funding Details"));
    dialog.resize(300, 200);

    QFormLayout* layout = new QFormLayout(&dialog);

    // Cost amount and currency
    QDoubleSpinBox* costSpinBox = new QDoubleSpinBox(&dialog);
    costSpinBox->setRange(0.01, 99999.99);
    costSpinBox->setDecimals(2);
    costSpinBox->setValue(fundingData.value("cost_amount", 20.0).toDouble());

    QComboBox* costCurrencyCombo = new QComboBox(&dialog);
    costCurrencyCombo->addItems(TBCurrency::supportedCodes());
    costCurrencyCombo->setCurrentText(fundingData.value("cost_currency", TBCurrency::defaultCurrencyCode()).toString());

    // Commission/Fee amount and currency
    QDoubleSpinBox* feeSpinBox = new QDoubleSpinBox(&dialog);
    feeSpinBox->setRange(0.0, 99999.99);
    feeSpinBox->setDecimals(2);
    feeSpinBox->setValue(fundingData.value("commission_amount", 0.0).toDouble());

    QComboBox* feeCurrencyCombo = new QComboBox(&dialog);
    feeCurrencyCombo->addItems(TBCurrency::supportedCodes());
    feeCurrencyCombo->setCurrentText(fundingData.value("commission_currency", TBCurrency::defaultCurrencyCode()).toString());

    // Equity amount (currency not editable, uses tournament payout currency)
    QDoubleSpinBox* equitySpinBox = new QDoubleSpinBox(&dialog);
    equitySpinBox->setRange(0.01, 99999.99);
    equitySpinBox->setDecimals(2);
    equitySpinBox->setValue(fundingData.value("equity_amount", 20.0).toDouble());

    QString payoutCurrency = TBCurrency::defaultCurrencyCode(); // Default
    // Get payout currency from parent widget if available
    QWidget* parentWidget = parent;
    while (parentWidget && !parentWidget->inherits("TBSetupFundingWidget"))
    {
        parentWidget = parentWidget->parentWidget();
    }
    if (parentWidget)
    {
        QComboBox* currencyCombo = parentWidget->findChild<QComboBox*>("payoutCurrencyComboBox");
        if (currencyCombo)
        {
            payoutCurrency = currencyCombo->currentText();
        }
    }
    QLabel* equityCurrencyLabel = new QLabel(payoutCurrency, &dialog);

    // Add fields to form
    layout->addRow(tr("Cost:"), costSpinBox);
    layout->addRow(tr("Cost Currency:"), costCurrencyCombo);
    layout->addRow(tr("Fee:"), feeSpinBox);
    layout->addRow(tr("Fee Currency:"), feeCurrencyCombo);
    layout->addRow(tr("Equity:"), equitySpinBox);
    layout->addRow(tr("Equity Currency:"), equityCurrencyLabel);

    // Dialog buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttonBox);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted)
    {
        fundingData["cost_amount"] = costSpinBox->value();
        fundingData["cost_currency"] = costCurrencyCombo->currentText();
        fundingData["commission_amount"] = feeSpinBox->value();
        fundingData["commission_currency"] = feeCurrencyCombo->currentText();
        fundingData["equity_amount"] = equitySpinBox->value();
        return true;
    }

    return false;
}

void TBFundingDetailsDelegate::onDetailsButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button)
        return;

    QModelIndex index = button->property("modelIndex").value<QModelIndex>();
    if (!index.isValid())
        return;

    QVariantMap rowData = index.model()->index(index.row(), 0).data(Qt::UserRole).toMap();
    if (showDetailsDialog(button, rowData))
    {
        button->setProperty("modifiedData", rowData);
    }
}