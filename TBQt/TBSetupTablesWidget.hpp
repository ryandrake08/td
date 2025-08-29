#pragma once

#include "TBSetupTabWidget.hpp"
#include <memory>

class TBSetupTablesWidget : public TBSetupTabWidget
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBSetupTablesWidget(QWidget* parent = nullptr);
    virtual ~TBSetupTablesWidget() override;

    void setConfiguration(const QVariantMap& configuration) override;
    QVariantMap configuration() const override;
    bool validateConfiguration() const override;

private Q_SLOTS:
    void on_addTableButtonClicked();
    void on_removeTableButtonClicked();
    void on_modelDataChanged();
};