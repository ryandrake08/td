#pragma once

#include "TBSetupTabWidget.hpp"
#include <memory>

class TBSetupFundingWidget : public TBSetupTabWidget
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBSetupFundingWidget(QWidget* parent = nullptr);
    virtual ~TBSetupFundingWidget() override;

    void setConfiguration(const QVariantMap& configuration) override;
    QVariantMap configuration() const override;
    bool validateConfiguration() const override;

    void setRoundsData(const QVariantList& rounds);

private Q_SLOTS:
    void on_addFundingButtonClicked();
    void on_removeFundingButtonClicked();
    void on_modelDataChanged();

private:
    QVariantMap createDefaultFunding(int fundingType) const;
    QString getDefaultCurrency() const;
};