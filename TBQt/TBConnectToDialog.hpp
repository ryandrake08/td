#pragma once

#include <QDialog>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class TBConnectToDialog; }
QT_END_NAMESPACE

class TBConnectToDialog : public QDialog
{
    Q_OBJECT

    std::unique_ptr<Ui::TBConnectToDialog> ui;

public:
    explicit TBConnectToDialog(QWidget* parent = nullptr);
    virtual ~TBConnectToDialog();

    // accessors
    QString host() const;
    void set_host(const QString& host);

    int port() const;
    void set_port(int port);

private Q_SLOTS:
    void on_hostEdit_textChanged();
    void updateConnectButtonState();
};