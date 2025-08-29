#pragma once

#include "TBSetupTabWidget.hpp"
#include <memory>

class TBSetupDevicesWidget : public TBSetupTabWidget
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBSetupDevicesWidget(QWidget* parent = nullptr);
    virtual ~TBSetupDevicesWidget() override;

    void setConfiguration(const QVariantMap& configuration) override;
    QVariantMap configuration() const override;
    bool validateConfiguration() const override;

private Q_SLOTS:
    void on_addDeviceButtonClicked();
    void on_removeDeviceButtonClicked();
    void on_modelDataChanged();

private:
    QVariantMap createNewDevice() const;
    int generateAuthorizationCode() const;
};