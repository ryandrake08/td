#pragma once

#include <QDialog>
#include <QVariant>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class TBMovementDialog; }
class QStandardItemModel;
QT_END_NAMESPACE

class TBMovementDialog : public QDialog
{
    Q_OBJECT

    std::unique_ptr<Ui::TBMovementDialog> ui;
    QStandardItemModel* model;

public:
    explicit TBMovementDialog(QWidget* parent = nullptr);
    virtual ~TBMovementDialog();

    // Set the movement data to display
    void setMovements(const QVariantList& movements);
};