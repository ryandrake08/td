#pragma once

#include <QDialog>
#include <memory>

class TBAuthCodeDialog : public QDialog
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBAuthCodeDialog(QWidget* parent = nullptr);
    ~TBAuthCodeDialog();

    int getAuthCode() const;

private Q_SLOTS:
    void onCodeFieldChanged();
    void onAccepted();
};