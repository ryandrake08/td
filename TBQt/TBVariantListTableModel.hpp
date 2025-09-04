#pragma once

#include <QAbstractTableModel>
#include <QObject>
#include <QString>
#include <memory>

class TBVariantListTableModel : public QAbstractTableModel
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBVariantListTableModel(QObject* parent = nullptr);
    virtual ~TBVariantListTableModel() override;

    // set data
    virtual void setListData(const QVariantList& model);
    virtual QVariantList listData() const;
    void addHeader(const QString& key, const QString& column);
    void addIndexHeader(const QString& key, const QString& column, int offset = 1);

    // header
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // basic functionality
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // editable
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // add data
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    // remove data
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

protected:
    // accessor for subclasses to get raw row data
    QVariantMap getRowData(int row) const;
};
