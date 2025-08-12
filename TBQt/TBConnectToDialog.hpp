#pragma once

#include <QDialog>
#include <memory>

class QLineEdit;
class QSpinBox;

class TBConnectToDialog : public QDialog
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBConnectToDialog(QWidget* parent = nullptr);
    virtual ~TBConnectToDialog();

    // accessors
    QString host() const;
    void set_host(const QString& host);
    
    int port() const;
    void set_port(int port);

private Q_SLOTS:
    void on_connectButton_clicked();
    void on_cancelButton_clicked();
    void on_host_textChanged();
    void update_connect_button_state();
};