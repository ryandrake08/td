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
    void on_addTurnoutPayoutButtonClicked();
    void on_removeTurnoutPayoutButtonClicked();
    void on_turnoutSelectionChanged();
    void on_turnoutPayoutSelectionChanged();
    void on_modelDataChanged();
    void on_percentSeatsSliderChanged(int value);
    void on_percentSeatsSpinBoxChanged(int value);
    void on_payoutShapeChanged(int value);
    void on_payoutTabChanged(int index);

private:
    void updateTurnoutPayoutsDisplay();
    QVariantMap createDefaultPayout(double amount) const;
    QVariantMap createDefaultTurnoutLevel(int buyinsCount) const;
    QString payoutShapeDescription(double shape) const;
};