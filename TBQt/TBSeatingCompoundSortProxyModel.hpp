#pragma once

#include <QSortFilterProxyModel>

class TBSeatingCompoundSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit TBSeatingCompoundSortProxyModel(QObject* parent = nullptr);

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
};