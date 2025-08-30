#include "TBSetupDialog.hpp"

#include "TBSetupPlayersWidget.hpp"
#include "TBSetupTablesWidget.hpp"
#include "TBSetupChipsWidget.hpp"
#include "TBSetupRoundsWidget.hpp"
#include "TBSetupFundingWidget.hpp"
#include "TBSetupPayoutsWidget.hpp"
#include "TBSetupDevicesWidget.hpp"

#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>

struct TBSetupDialog::impl
{
    QTabWidget* tabWidget;
    QDialogButtonBox* buttonBox;
    QVariantMap configuration;

    TBSetupPlayersWidget* playersTab;
    TBSetupTablesWidget* tablesTab;
    TBSetupChipsWidget* chipsTab;
    TBSetupRoundsWidget* roundsTab;
    TBSetupFundingWidget* fundingTab;
    TBSetupPayoutsWidget* payoutsTab;
    TBSetupDevicesWidget* devicesTab;
};

TBSetupDialog::TBSetupDialog(QWidget* parent) : QDialog(parent), pimpl(new impl())
{
    setWindowTitle(tr("Tournament Setup"));
    setModal(true);
    resize(700, 500);

    pimpl->configuration = QVariantMap();

    // Create main layout
    auto* mainLayout = new QVBoxLayout(this);

    // Create tab widget with 7 tabs matching macOS structure
    pimpl->tabWidget = new QTabWidget(this);

    // Create players tab
    pimpl->playersTab = new TBSetupPlayersWidget(this);
    pimpl->tabWidget->addTab(pimpl->playersTab, tr("Players"));

    // Create chips tab
    pimpl->chipsTab = new TBSetupChipsWidget(this);
    pimpl->tabWidget->addTab(pimpl->chipsTab, tr("Chips"));

    // Create tables tab
    pimpl->tablesTab = new TBSetupTablesWidget(this);
    pimpl->tabWidget->addTab(pimpl->tablesTab, tr("Tables"));

    // Create funding tab
    pimpl->fundingTab = new TBSetupFundingWidget(this);
    pimpl->tabWidget->addTab(pimpl->fundingTab, tr("Funding"));

    // Create rounds tab
    pimpl->roundsTab = new TBSetupRoundsWidget(this);
    pimpl->tabWidget->addTab(pimpl->roundsTab, tr("Rounds"));

    // Create payouts tab
    pimpl->payoutsTab = new TBSetupPayoutsWidget(this);
    pimpl->tabWidget->addTab(pimpl->payoutsTab, tr("Payouts"));

    // Create devices tab
    pimpl->devicesTab = new TBSetupDevicesWidget(this);
    pimpl->tabWidget->addTab(pimpl->devicesTab, tr("Devices"));

    mainLayout->addWidget(pimpl->tabWidget);

    // Create OK/Cancel buttons
    pimpl->buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(pimpl->buttonBox);

    // Connect button signals
    connect(pimpl->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(pimpl->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Connect rounds tab changes to update funding tab
    connect(pimpl->roundsTab, &TBSetupTabWidget::configurationChanged, this, [this]() {
        // Update funding tab with latest rounds data when rounds configuration changes
        QVariantMap roundsConfig = pimpl->roundsTab->configuration();
        QVariantList rounds = roundsConfig.value("blind_levels").toList();
        pimpl->fundingTab->setRoundsData(rounds);
    });
}

TBSetupDialog::~TBSetupDialog()
{
}

void TBSetupDialog::setConfiguration(const QVariantMap& configuration)
{
    pimpl->configuration = configuration;

    // Set configuration for each tab
    pimpl->playersTab->setConfiguration(configuration);
    pimpl->tablesTab->setConfiguration(configuration);
    pimpl->chipsTab->setConfiguration(configuration);
    pimpl->roundsTab->setConfiguration(configuration);
    pimpl->fundingTab->setConfiguration(configuration);
    pimpl->payoutsTab->setConfiguration(configuration);
    pimpl->devicesTab->setConfiguration(configuration);

    // Update funding tab with rounds data for blind level dropdown
    QVariantList rounds = configuration.value("blind_levels").toList();
    pimpl->fundingTab->setRoundsData(rounds);
}

QVariantMap TBSetupDialog::configuration() const
{
    QVariantMap config = pimpl->configuration;

    // Merge configuration from each tab
    QVariantMap playersConfig = pimpl->playersTab->configuration();
    for (auto it = playersConfig.begin(); it != playersConfig.end(); ++it)
    {
        config[it.key()] = it.value();
    }

    QVariantMap tablesConfig = pimpl->tablesTab->configuration();
    for (auto it = tablesConfig.begin(); it != tablesConfig.end(); ++it)
    {
        config[it.key()] = it.value();
    }

    QVariantMap chipsConfig = pimpl->chipsTab->configuration();
    for (auto it = chipsConfig.begin(); it != chipsConfig.end(); ++it)
    {
        config[it.key()] = it.value();
    }

    QVariantMap roundsConfig = pimpl->roundsTab->configuration();
    for (auto it = roundsConfig.begin(); it != roundsConfig.end(); ++it)
    {
        config[it.key()] = it.value();
    }

    QVariantMap fundingConfig = pimpl->fundingTab->configuration();
    for (auto it = fundingConfig.begin(); it != fundingConfig.end(); ++it)
    {
        config[it.key()] = it.value();
    }

    QVariantMap payoutsConfig = pimpl->payoutsTab->configuration();
    for (auto it = payoutsConfig.begin(); it != payoutsConfig.end(); ++it)
    {
        config[it.key()] = it.value();
    }

    QVariantMap devicesConfig = pimpl->devicesTab->configuration();
    for (auto it = devicesConfig.begin(); it != devicesConfig.end(); ++it)
    {
        config[it.key()] = it.value();
    }

    return config;
}