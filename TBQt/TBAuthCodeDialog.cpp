#include "TBAuthCodeDialog.hpp"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExpValidator>
#include <QVBoxLayout>

struct TBAuthCodeDialog::impl
{
    QLineEdit* codeFields[5];
    QPushButton* okButton;
    QPushButton* cancelButton;
    QLabel* instructionLabel;
    int authCode;

    explicit impl() : okButton(nullptr), cancelButton(nullptr), instructionLabel(nullptr), authCode(0)
    {
        // Initialize code fields
        for(int i = 0; i < 5; ++i)
        {
            codeFields[i] = nullptr;
        }
    }
};

TBAuthCodeDialog::TBAuthCodeDialog(QWidget* parent) : QDialog(parent), pimpl(std::unique_ptr<impl>(new impl()))
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Instruction label
    pimpl->instructionLabel = new QLabel(tr("Enter 5-digit authorization code:"), this);
    pimpl->instructionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(pimpl->instructionLabel);

    // Code fields layout
    QHBoxLayout* codeLayout = new QHBoxLayout();
    codeLayout->setSpacing(5);

    // Create 5 single-digit fields
    QRegExp digitRegex("[0-9]");
    QRegExpValidator* digitValidator = new QRegExpValidator(digitRegex, this);

    for(int i = 0; i < 5; ++i)
    {
        pimpl->codeFields[i] = new QLineEdit(this);
        pimpl->codeFields[i]->setMaxLength(1);
        pimpl->codeFields[i]->setValidator(digitValidator);
        pimpl->codeFields[i]->setFixedSize(30, 30);
        pimpl->codeFields[i]->setAlignment(Qt::AlignCenter);
        pimpl->codeFields[i]->setStyleSheet("QLineEdit { font-size: 14px; font-weight: bold; }");

        codeLayout->addWidget(pimpl->codeFields[i]);
    }

    mainLayout->addLayout(codeLayout);

    // Add some spacing before buttons
    mainLayout->addSpacing(20);

    // Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    pimpl->okButton = buttonBox->button(QDialogButtonBox::Ok);
    pimpl->cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

    pimpl->okButton->setText(tr("Authorize"));
    pimpl->okButton->setEnabled(false); // Initially disabled

    mainLayout->addWidget(buttonBox);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &TBAuthCodeDialog::onAccepted);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // connect signals
    for(int i = 0; i < 5; ++i)
    {
        QObject::connect(pimpl->codeFields[i], &QLineEdit::textChanged, this, &TBAuthCodeDialog::onCodeFieldChanged);
    }

    // Set initial UI state
    setWindowTitle(tr("Authorization Code"));
    setModal(true);
    setFixedSize(300, 150);

    // Focus first field
    pimpl->codeFields[0]->setFocus();
}

TBAuthCodeDialog::~TBAuthCodeDialog() = default;

void TBAuthCodeDialog::onCodeFieldChanged()
{
    QLineEdit* sender = qobject_cast<QLineEdit*>(QObject::sender());
    if(!sender)
        return;

    // Find which field sent the signal
    int currentIndex = -1;
    for(int i = 0; i < 5; ++i)
    {
        if(pimpl->codeFields[i] == sender)
        {
            currentIndex = i;
            break;
        }
    }

    if(currentIndex == -1)
        return;

    // If field has text and it's not the last field, move to next
    if(!sender->text().isEmpty() && currentIndex < 4)
    {
        pimpl->codeFields[currentIndex + 1]->setFocus();
        pimpl->codeFields[currentIndex + 1]->selectAll();
    }

    // Check if all fields are filled
    bool allFilled = true;
    for(int i = 0; i < 5; ++i)
    {
        if(pimpl->codeFields[i]->text().isEmpty())
        {
            allFilled = false;
            break;
        }
    }

    pimpl->okButton->setEnabled(allFilled);

    // If all filled, auto-submit
    if(allFilled)
    {
        onAccepted();
    }
}

void TBAuthCodeDialog::onAccepted()
{
    // Build the 5-digit code
    QString codeString;
    for(int i = 0; i < 5; ++i)
    {
        codeString += pimpl->codeFields[i]->text();
    }

    bool ok;
    pimpl->authCode = codeString.toInt(&ok);

    if(ok && pimpl->authCode >= 10000 && pimpl->authCode <= 99999)
    {
        accept();
    }
    else
    {
        // Should not happen with validation, but just in case
        pimpl->authCode = 0;
    }
}

int TBAuthCodeDialog::getAuthCode() const
{
    return pimpl->authCode;
}