#pragma once

#include "TBSetupTabWidget.hpp"
#include <memory>

class TBSetupChipsWidget : public TBSetupTabWidget
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBSetupChipsWidget(QWidget* parent = nullptr);
    virtual ~TBSetupChipsWidget() override;

    void setConfiguration(const QVariantMap& configuration) override;
    QVariantMap configuration() const override;
    bool validateConfiguration() const override;

private Q_SLOTS:
    void on_addChipButtonClicked();
    void on_removeChipButtonClicked();
    void on_modelDataChanged();

private:
    QString generateRandomColor() const;
    QList<int> getDefaultDenominations() const;
};