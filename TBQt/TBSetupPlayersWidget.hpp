#pragma once

#include "TBSetupTabWidget.hpp"
#include <memory>

class QTableView;
class QAbstractItemModel;

class TBSetupPlayersWidget : public TBSetupTabWidget
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBSetupPlayersWidget(QWidget* parent = nullptr);
    virtual ~TBSetupPlayersWidget() override;

    void setConfiguration(const QVariantMap& configuration) override;
    QVariantMap configuration() const override;
    bool validateConfiguration() const override;

private Q_SLOTS:
    void on_addPlayerButtonClicked();
    void on_removePlayerButtonClicked();
    void on_modelDataChanged();
};