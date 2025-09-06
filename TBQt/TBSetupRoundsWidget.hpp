#pragma once

#include "TBSetupTabWidget.hpp"
#include <memory>

class TBSetupRoundsWidget : public TBSetupTabWidget
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

    QVariantMap createDefaultRound(int littleBlind, int bigBlind) const;
    int calculateNextBlindLevel() const;

public:
    explicit TBSetupRoundsWidget(QWidget* parent = nullptr);
    virtual ~TBSetupRoundsWidget() override;

    void setConfiguration(const QVariantMap& configuration) override;
    QVariantMap configuration() const override;
    bool validateConfiguration() const override;

private Q_SLOTS:
    void on_addRoundButtonClicked();
    void on_generateButtonClicked();
    void on_removeButtonClicked();
    void on_modelDataChanged();
};