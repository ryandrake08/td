#pragma once

#include "TBSetupTabWidget.hpp"
#include <memory>

class TBSetupPayoutsWidget : public TBSetupTabWidget
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBSetupPayoutsWidget(QWidget* parent = nullptr);
    virtual ~TBSetupPayoutsWidget() override;

    void setConfiguration(const QVariantMap& configuration) override;
    QVariantMap configuration() const override;
    bool validateConfiguration() const override;

private Q_SLOTS:
    void on_addPayoutButtonClicked();
    void on_removePayoutButtonClicked();
    void on_addTurnoutButtonClicked();
    void on_removeTurnoutButtonClicked();
    void on_turnoutSelectionChanged();
    void on_modelDataChanged();

private:
    void updateTurnoutPayoutsDisplay();
    QVariantMap createDefaultPayout(int place, double amount) const;
    QVariantMap createDefaultTurnoutLevel(int buyinsCount) const;
};