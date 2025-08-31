#include "TBSetupDialog.hpp"

#include "TBSetupPlayersWidget.hpp"
#include "TBSetupTablesWidget.hpp"
#include "TBSetupChipsWidget.hpp"
#include "TBSetupRoundsWidget.hpp"
#include "TBSetupFundingWidget.hpp"
#include "TBSetupPayoutsWidget.hpp"
#include "TBSetupDevicesWidget.hpp"

#include "ui_TBSetupDialog.h"

struct TBSetupDialog::impl
{
    Ui::TBSetupDialog ui;
    QVariantMap configuration;

    // Store tabs in a list for easier iteration
    QList<TBSetupTabWidget*> tabs;
};

TBSetupDialog::TBSetupDialog(QWidget* parent) : QDialog(parent), pimpl(new impl())
{
    pimpl->configuration = QVariantMap();

    // Setup UI from .ui file
    pimpl->ui.setupUi(this);

    // Create all tabs using helper function
    auto addTab = [this](TBSetupTabWidget* tab, const QString& title)
    {
        pimpl->tabs.append(tab);
        pimpl->ui.tabWidget->addTab(tab, title);
        connect(tab, &TBSetupTabWidget::configurationChanged, this, &TBSetupDialog::onTabConfigurationChanged);
        return tab;
    };

    addTab(new TBSetupPlayersWidget(this), tr("Players"));
    addTab(new TBSetupChipsWidget(this), tr("Chips"));
    addTab(new TBSetupTablesWidget(this), tr("Tables"));
    addTab(new TBSetupFundingWidget(this), tr("Funding"));
    addTab(new TBSetupRoundsWidget(this), tr("Rounds"));
    addTab(new TBSetupPayoutsWidget(this), tr("Payouts"));
    addTab(new TBSetupDevicesWidget(this), tr("Devices"));
}

TBSetupDialog::~TBSetupDialog()
{
}

void TBSetupDialog::setConfiguration(const QVariantMap& configuration)
{
    pimpl->configuration = configuration;

    // Set tournament name
    pimpl->ui.tournamentNameEdit->setText(configuration.value("name", tr("Untitled Tournament")).toString());

    // Set configuration for each tab
    for(auto* tab : pimpl->tabs)
    {
        tab->setConfiguration(configuration);
    }

    // Trigger broadcast to handle any inter-tab dependencies
    for(auto* tab : pimpl->tabs)
    {
        tab->onOtherTabConfigurationChanged(configuration);
    }
}

QVariantMap TBSetupDialog::configuration() const
{
    QVariantMap config = pimpl->configuration;

    // Set tournament name
    config["name"] = pimpl->ui.tournamentNameEdit->text();

    // Merge configuration from each tab
    for(auto* tab : pimpl->tabs)
    {
        QVariantMap tabConfig = tab->configuration();
        for (auto it = tabConfig.begin(); it != tabConfig.end(); ++it)
        {
            config[it.key()] = it.value();
        }
    }

    return config;
}

void TBSetupDialog::onTabConfigurationChanged()
{
    // Get current full configuration
    QVariantMap currentConfig = this->configuration();

    // Broadcast to all tabs
    for(auto* tab : pimpl->tabs)
    {
        tab->onOtherTabConfigurationChanged(currentConfig);
    }
}
