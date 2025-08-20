#include "TBConnectToDialog.hpp"
#include "TournamentService.hpp"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

struct TBConnectToDialog::impl
{
    QLineEdit* host_edit;
    QSpinBox* port_spinbox;
    QPushButton* connect_button;
    QPushButton* cancel_button;
    QDialogButtonBox* button_box;
};

TBConnectToDialog::TBConnectToDialog(QWidget* parent) : QDialog(parent), pimpl(new impl())
{
    setWindowTitle("Connect to Tournament");
    setModal(true);
    resize(350, 150);

    // create layout
    auto main_layout = new QVBoxLayout(this);
    auto form_layout = new QFormLayout();

    // host input
    pimpl->host_edit = new QLineEdit(this);
    pimpl->host_edit->setPlaceholderText("Tournament server address");
    form_layout->addRow("Host:", pimpl->host_edit);

    // port input
    pimpl->port_spinbox = new QSpinBox(this);
    pimpl->port_spinbox->setRange(1, 65535);
    pimpl->port_spinbox->setValue(TournamentService::default_port);
    form_layout->addRow("Port:", pimpl->port_spinbox);

    main_layout->addLayout(form_layout);

    // button box
    pimpl->button_box = new QDialogButtonBox(this);
    pimpl->connect_button = pimpl->button_box->addButton("Connect", QDialogButtonBox::AcceptRole);
    pimpl->cancel_button = pimpl->button_box->addButton("Cancel", QDialogButtonBox::RejectRole);

    main_layout->addWidget(pimpl->button_box);

    // initially disable connect button until host is entered
    pimpl->connect_button->setEnabled(false);

    // connect signals
    connect(pimpl->connect_button, &QPushButton::clicked, this, &TBConnectToDialog::on_connectButton_clicked);
    connect(pimpl->cancel_button, &QPushButton::clicked, this, &TBConnectToDialog::on_cancelButton_clicked);
    connect(pimpl->host_edit, &QLineEdit::textChanged, this, &TBConnectToDialog::on_host_textChanged);

    // focus host edit
    pimpl->host_edit->setFocus();
}

TBConnectToDialog::~TBConnectToDialog() = default;

QString TBConnectToDialog::host() const
{
    return pimpl->host_edit->text().trimmed();
}

void TBConnectToDialog::set_host(const QString& host)
{
    pimpl->host_edit->setText(host);
}

int TBConnectToDialog::port() const
{
    return pimpl->port_spinbox->value();
}

void TBConnectToDialog::set_port(int port)
{
    pimpl->port_spinbox->setValue(port);
}

void TBConnectToDialog::on_connectButton_clicked()
{
    accept();
}

void TBConnectToDialog::on_cancelButton_clicked()
{
    reject();
}

void TBConnectToDialog::on_host_textChanged()
{
    update_connect_button_state();
}

void TBConnectToDialog::update_connect_button_state()
{
    bool enabled = !host().isEmpty();
    pimpl->connect_button->setEnabled(enabled);
}