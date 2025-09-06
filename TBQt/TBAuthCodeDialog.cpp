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
#include <algorithm>
#include <array>

struct TBAuthCodeDialog::impl
{
    std::array<QLineEdit*, 5> codeFields {};
    QPushButton* okButton { nullptr };
    QPushButton* cancelButton { nullptr };
    QLabel* instructionLabel { nullptr };
    int authCode { 0 };

    explicit impl()
    {
        // Initialize code fields
        for(auto& codeField : codeFields)
        {
            codeField = nullptr;
        }
    }
};

TBAuthCodeDialog::TBAuthCodeDialog(QWidget* parent) : QDialog(parent), pimpl(std::unique_ptr<impl>(new impl()))
{
    auto* mainLayout = new QVBoxLayout(this);

    // Instruction label
    pimpl->instructionLabel = new QLabel(tr("Enter 5-digit authorization code:"), this);
    pimpl->instructionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(pimpl->instructionLabel);

    // Code fields layout
    auto* codeLayout = new QHBoxLayout();
    codeLayout->setSpacing(5);

    // Create 5 single-digit fields
    QRegExp digitRegex("[0-9]");
    auto* digitValidator = new QRegExpValidator(digitRegex, this);

    for(auto& codeField : pimpl->codeFields)
    {
        codeField = new QLineEdit(this);
        codeField->setMaxLength(1);
        codeField->setValidator(digitValidator);
        codeField->setFixedSize(30, 30);
        codeField->setAlignment(Qt::AlignCenter);
        codeField->setStyleSheet("QLineEdit { font-size: 14px; font-weight: bold; }");

        codeLayout->addWidget(codeField);
    }

    mainLayout->addLayout(codeLayout); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)

    // Add some spacing before buttons
    mainLayout->addSpacing(20);

    // Buttons
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    pimpl->okButton = buttonBox->button(QDialogButtonBox::Ok);
    pimpl->cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

    pimpl->okButton->setText(tr("Authorize"));
    pimpl->okButton->setEnabled(false); // Initially disabled

    mainLayout->addWidget(buttonBox);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &TBAuthCodeDialog::onAccepted);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // connect signals
    for(auto& codeField : pimpl->codeFields)
    {
        QObject::connect(codeField, &QLineEdit::textChanged, this, &TBAuthCodeDialog::onCodeFieldChanged);
    }

    // Set initial UI state
    setWindowTitle(tr("Authorization Code"));
    setModal(true);
    setFixedSize(300, 150);

    // Focus first field
    pimpl->codeFields.at(0)->setFocus();
}

TBAuthCodeDialog::~TBAuthCodeDialog() = default;

void TBAuthCodeDialog::onCodeFieldChanged()
{
    auto* sender = qobject_cast<QLineEdit*>(QObject::sender());
    if(!sender)
        return;

    // Find which field sent the signal
    auto* it = std::find(pimpl->codeFields.begin(), pimpl->codeFields.end(), sender);
    if(it == pimpl->codeFields.end())
        return;

    int currentIndex = std::distance(pimpl->codeFields.begin(), it);

    // If field has text and it's not the last field, move to next
    if(!sender->text().isEmpty() && currentIndex < 4)
    {
        pimpl->codeFields.at(currentIndex + 1)->setFocus();
        pimpl->codeFields.at(currentIndex + 1)->selectAll();
    }

    // Check if all fields are filled
    bool allFilled = std::all_of(pimpl->codeFields.begin(), pimpl->codeFields.end(),
                                 [](const QLineEdit* field)
    {
        return !field->text().isEmpty();
    });

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
    for(auto& codeField : pimpl->codeFields)
    {
        codeString += codeField->text();
    }

    bool ok = false;
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