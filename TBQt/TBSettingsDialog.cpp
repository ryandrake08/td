#include "TBSettingsDialog.hpp"

#include "ui_TBSettingsDialog.h"

#include <QApplication>
#include <QScreen>
#include <QSettings>

struct TBSettingsDialog::impl
{
    Ui::TBSettingsDialog ui {};

    explicit impl(TBSettingsDialog* parent)
    {
        ui.setupUi(parent);
    }
};

TBSettingsDialog::TBSettingsDialog(QWidget* parent) : QDialog(parent), pimpl(new impl(this))
{
    // Populate screen selection comboboxes
    QList<QScreen*> screens = QApplication::screens();

    for(int i = 0; i < screens.size(); ++i)
    {
        QScreen* screen = screens[i];
        QString screenName = QString("Screen %1").arg(i + 1);
        if(i == 0)
        {
            screenName += " (Primary)";
        }

        // Add screen geometry info for clarity
        QRect geometry = screen->geometry();
        QString description = QString("%1 (%2x%3)").arg(screenName).arg(geometry.width()).arg(geometry.height());

        pimpl->ui.tournamentScreenComboBox->addItem(description, i);
        pimpl->ui.seatingScreenComboBox->addItem(description, i);
    }

    // Load current settings
    QSettings settings;

    // Tournament Display settings
    bool tournamentFullscreen = settings.value("Display/TournamentDisplayFullscreen", false).toBool();
    int tournamentScreen = settings.value("Display/TournamentDisplayScreen", 0).toInt();

    pimpl->ui.tournamentFullscreenCheckBox->setChecked(tournamentFullscreen);

    // Validate screen index and set combobox
    if(tournamentScreen >= 0 && tournamentScreen < screens.size())
    {
        pimpl->ui.tournamentScreenComboBox->setCurrentIndex(tournamentScreen);
    }
    else
    {
        pimpl->ui.tournamentScreenComboBox->setCurrentIndex(0);
    }

    // Seating Chart settings
    bool seatingFullscreen = settings.value("Display/SeatingChartFullscreen", false).toBool();
    int seatingScreen = settings.value("Display/SeatingChartScreen", 0).toInt();

    pimpl->ui.seatingFullscreenCheckBox->setChecked(seatingFullscreen);

    // Validate screen index and set combobox
    if(seatingScreen >= 0 && seatingScreen < screens.size())
    {
        pimpl->ui.seatingScreenComboBox->setCurrentIndex(seatingScreen);
    }
    else
    {
        pimpl->ui.seatingScreenComboBox->setCurrentIndex(0);
    }

    // Connect button box signals
    QObject::connect(pimpl->ui.buttonBox, &QDialogButtonBox::accepted, this, &TBSettingsDialog::on_buttonBox_accepted);
    QObject::connect(pimpl->ui.buttonBox, &QDialogButtonBox::rejected, this, &TBSettingsDialog::on_buttonBox_rejected);
}

TBSettingsDialog::~TBSettingsDialog() = default;

void TBSettingsDialog::on_buttonBox_accepted()
{
    // Save settings to QSettings
    QSettings settings;

    // Tournament Display settings
    settings.setValue("Display/TournamentDisplayFullscreen", pimpl->ui.tournamentFullscreenCheckBox->isChecked());
    settings.setValue("Display/TournamentDisplayScreen", pimpl->ui.tournamentScreenComboBox->currentData().toInt());

    // Seating Chart settings
    settings.setValue("Display/SeatingChartFullscreen", pimpl->ui.seatingFullscreenCheckBox->isChecked());
    settings.setValue("Display/SeatingChartScreen", pimpl->ui.seatingScreenComboBox->currentData().toInt());

    accept();
}

void TBSettingsDialog::on_buttonBox_rejected()
{
    reject();
}