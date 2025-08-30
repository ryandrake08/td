#pragma once

#include <QDialog>
#include <QVariantMap>
#include <memory>

class TBSetupDialog : public QDialog
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBSetupDialog(QWidget* parent = nullptr);
    virtual ~TBSetupDialog() override;

    void setConfiguration(const QVariantMap& configuration);
    QVariantMap configuration() const;

private Q_SLOTS:
    void onRoundsConfigurationChanged();
};