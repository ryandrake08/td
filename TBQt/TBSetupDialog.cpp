#include "TBSetupDialog.hpp"

#include "TBSetupPlayersWidget.hpp"

#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>

struct TBSetupDialog::impl
{
    QTabWidget* tabWidget;
    QDialogButtonBox* buttonBox;
    QVariantMap configuration;
    
    TBSetupPlayersWidget* playersTab;
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
    
    // Create placeholder widgets for remaining tabs
    // These will be replaced with proper implementations in subsequent stages
    pimpl->tabWidget->addTab(new QWidget(), tr("Tables"));
    pimpl->tabWidget->addTab(new QWidget(), tr("Chips"));
    pimpl->tabWidget->addTab(new QWidget(), tr("Rounds"));
    pimpl->tabWidget->addTab(new QWidget(), tr("Funding"));
    pimpl->tabWidget->addTab(new QWidget(), tr("Payouts"));
    pimpl->tabWidget->addTab(new QWidget(), tr("Devices"));

    mainLayout->addWidget(pimpl->tabWidget);

    // Create OK/Cancel buttons
    pimpl->buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(pimpl->buttonBox);

    // Connect button signals
    connect(pimpl->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(pimpl->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

TBSetupDialog::~TBSetupDialog()
{
}

void TBSetupDialog::setConfiguration(const QVariantMap& configuration)
{
    pimpl->configuration = configuration;
    
    // Set configuration for each tab
    pimpl->playersTab->setConfiguration(configuration);
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
    
    return config;
}