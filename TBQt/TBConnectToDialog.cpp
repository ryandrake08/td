#include "TBConnectToDialog.hpp"
#include "ui_TBConnectToDialog.h"
#include "TournamentService.hpp"

#include <QPushButton>

TBConnectToDialog::TBConnectToDialog(QWidget* parent) : QDialog(parent), ui(new Ui::TBConnectToDialog)
{
    ui->setupUi(this);
    
    // Set default port value
    ui->portSpinBox->setValue(TournamentService::default_port);
    
    // Initially disable OK button until host is entered
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    
    // Connect signals
    connect(ui->hostEdit, &QLineEdit::textChanged, this, &TBConnectToDialog::on_hostEdit_textChanged);
    
    // Focus host edit
    ui->hostEdit->setFocus();
}

TBConnectToDialog::~TBConnectToDialog() = default;

QString TBConnectToDialog::host() const
{
    return ui->hostEdit->text().trimmed();
}

void TBConnectToDialog::set_host(const QString& host)
{
    ui->hostEdit->setText(host);
}

int TBConnectToDialog::port() const
{
    return ui->portSpinBox->value();
}

void TBConnectToDialog::set_port(int port)
{
    ui->portSpinBox->setValue(port);
}

void TBConnectToDialog::on_hostEdit_textChanged()
{
    updateConnectButtonState();
}

void TBConnectToDialog::updateConnectButtonState()
{
    bool enabled = !host().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}