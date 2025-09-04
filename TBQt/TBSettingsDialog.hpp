#pragma once

#include <QDialog>
#include <memory>

class TBSettingsDialog : public QDialog
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBSettingsDialog(QWidget* parent = nullptr);
    ~TBSettingsDialog();

private Q_SLOTS:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
};