#pragma once

#include "TBSetupTabWidget.hpp"

#include <QVariantList>
#include <QVariantMap>
#include <functional>
#include <memory>

// Callback type for generating blind levels
// Takes: request parameters (QVariantMap), handler for results (QVariantList)
using BlindLevelGenerator = std::function<void(const QVariantMap&, std::function<void(const QVariantList&)>)>;

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

    // Set the blind level generator callback
    void setBlindLevelGenerator(BlindLevelGenerator generator);

private Q_SLOTS:
    void on_addRoundButtonClicked();
    void on_generateButtonClicked();
    void on_removeButtonClicked();
    void on_modelDataChanged();
};