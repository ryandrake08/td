#pragma once

#include "TBVariantListTableModel.hpp"

class TBRoundsModel : public TBVariantListTableModel
{
    Q_OBJECT

public:
    explicit TBRoundsModel(QObject* parent = nullptr);

    void setListData(const QVariantList& model) override;
    QVariantList listData() const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    QString formatTime(int minutes) const;
    QString calculateStartTime(int roundIndex) const;
    QVariantList m_originalData;
};