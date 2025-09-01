#pragma once

#include <QDialog>
#include <QVariant>
#include <memory>

class QStandardItemModel;

class TBMovementDialog : public QDialog
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBMovementDialog(QWidget* parent = nullptr);
    virtual ~TBMovementDialog();

    // Set the movement data to display
    void setMovements(const QVariantList& movements);
};