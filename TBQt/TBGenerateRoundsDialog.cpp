#include "TBGenerateRoundsDialog.hpp"

#include "ui_TBGenerateRoundsDialog.h"

#include <QObject>

struct TBGenerateRoundsDialog::impl
{
    Ui::TBGenerateRoundsDialog ui {};
};

TBGenerateRoundsDialog::TBGenerateRoundsDialog(QWidget* parent) : QDialog(parent), pimpl(new impl())
{
    pimpl->ui.setupUi(this);

    // Connect ante type combo box to slot
    QObject::connect(pimpl->ui.anteTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     this, &TBGenerateRoundsDialog::on_anteTypeChanged);

    // Initialize ante ratio state based on initial ante type
    on_anteTypeChanged(pimpl->ui.anteTypeComboBox->currentIndex());
}

TBGenerateRoundsDialog::~TBGenerateRoundsDialog() = default;

int TBGenerateRoundsDialog::desiredDurationMs() const
{
    return pimpl->ui.desiredDurationSpinBox->value() * 60 * 1000;
}

int TBGenerateRoundsDialog::levelDurationMs() const
{
    return pimpl->ui.levelDurationSpinBox->value() * 60 * 1000;
}

int TBGenerateRoundsDialog::expectedBuyins() const
{
    return pimpl->ui.buyinsSpinBox->value();
}

int TBGenerateRoundsDialog::expectedRebuys() const
{
    return pimpl->ui.rebuysSpinBox->value();
}

int TBGenerateRoundsDialog::expectedAddons() const
{
    return pimpl->ui.addonsSpinBox->value();
}

int TBGenerateRoundsDialog::breakDurationMs() const
{
    return pimpl->ui.breakDurationSpinBox->value() * 60 * 1000;
}

int TBGenerateRoundsDialog::anteType() const
{
    return pimpl->ui.anteTypeComboBox->currentIndex();
}

double TBGenerateRoundsDialog::anteSbRatio() const
{
    return pimpl->ui.anteRatioSpinBox->value();
}

void TBGenerateRoundsDialog::on_anteTypeChanged(int index)
{
    // Enable ante ratio controls only when ante type is Traditional (index 1)
    bool enableRatio = (index == 1);

    pimpl->ui.anteRatioLabel->setEnabled(enableRatio);
    pimpl->ui.anteRatioSpinBox->setEnabled(enableRatio);
}
