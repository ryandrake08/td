#include "TBSetupDevicesWidget.hpp"

#include "TBDateEditDelegate.hpp"
#include "TBTableViewUtils.hpp"
#include "TBVariantListTableModel.hpp"

#include "ui_TBSetupDevicesWidget.h"

#include <QDateTime>
#include <QHeaderView>
#include <QRandomGenerator>
#include <QSortFilterProxyModel>

struct TBSetupDevicesWidget::impl
{
    Ui::TBSetupDevicesWidget ui;
    TBVariantListTableModel* model;
};

TBSetupDevicesWidget::TBSetupDevicesWidget(QWidget* parent) : TBSetupTabWidget(parent), pimpl(new impl())
{
    // Setup UI from .ui file
    pimpl->ui.setupUi(this);

    // Create and configure model
    pimpl->model = new TBVariantListTableModel(this);
    pimpl->model->addHeader("code", tr("Code"));
    pimpl->model->addHeader("name", tr("Device Name"));
    pimpl->model->addHeader("added_at", tr("Authorized"));

    // Set up table view with sorting
    TBTableViewUtils::setupTableViewWithSorting(this, pimpl->ui.tableView, pimpl->model, 1, Qt::AscendingOrder);

    // Configure column behavior
    QHeaderView* header = pimpl->ui.tableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Code
    header->setSectionResizeMode(1, QHeaderView::Stretch);          // Device Name
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Authorized

    // Set date delegate for Authorized column (column 2)
    pimpl->ui.tableView->setItemDelegateForColumn(2, new TBDateEditDelegate(this));

    // Connect signals
    QObject::connect(pimpl->ui.addButton, &QPushButton::clicked, this, &TBSetupDevicesWidget::on_addDeviceButtonClicked);
    QObject::connect(pimpl->ui.removeButton, &QPushButton::clicked, this, &TBSetupDevicesWidget::on_removeDeviceButtonClicked);
    QObject::connect(pimpl->model, &QAbstractItemModel::dataChanged, this, &TBSetupDevicesWidget::on_modelDataChanged);

    // Connect selection model after setting the model
    QObject::connect(pimpl->ui.tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
                     [this]()
    {
        bool hasSelection = pimpl->ui.tableView->selectionModel()->hasSelection();
        pimpl->ui.removeButton->setEnabled(hasSelection);
    });
}

TBSetupDevicesWidget::~TBSetupDevicesWidget() = default;

void TBSetupDevicesWidget::setConfiguration(const QVariantMap& configuration)
{
    QVariantList devices = configuration.value("authorized_clients").toList();
    pimpl->model->setListData(devices);
}

QVariantMap TBSetupDevicesWidget::configuration() const
{
    QVariantMap config;
    config["authorized_clients"] = pimpl->model->listData();
    return config;
}

bool TBSetupDevicesWidget::validateConfiguration() const
{
    // Devices are optional
    return true;
}

void TBSetupDevicesWidget::on_addDeviceButtonClicked()
{
    QVariantMap newDevice = createNewDevice();

    // Add to model
    QVariantList devices = pimpl->model->listData();
    devices.append(newDevice);
    pimpl->model->setListData(devices);

    Q_EMIT configurationChanged();
}

void TBSetupDevicesWidget::on_removeDeviceButtonClicked()
{
    int row = TBTableViewUtils::getSelectedSourceRow(pimpl->ui.tableView);
    if(row < 0)
        return;

    // Remove from model
    QVariantList devices = pimpl->model->listData();
    if(row >= 0 && row < devices.size())
    {
        devices.removeAt(row);
        pimpl->model->setListData(devices);
        Q_EMIT configurationChanged();
    }
}

void TBSetupDevicesWidget::on_modelDataChanged()
{
    Q_EMIT configurationChanged();
}

QVariantMap TBSetupDevicesWidget::createNewDevice() const
{
    QVariantMap device;
    device["code"] = generateAuthorizationCode();
    device["name"] = tr("Remote Device");
    device["added_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    return device;
}

int TBSetupDevicesWidget::generateAuthorizationCode() const
{
    // Generate 6-digit authorization code
    return QRandomGenerator::global()->bounded(100000, 999999);
}