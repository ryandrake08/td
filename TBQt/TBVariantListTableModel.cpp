#include "TBVariantListTableModel.hpp"

#include <QDebug>
#include <QList>
#include <QVariant>
#include <QVector>

struct TBVariantListTableModel::impl
{
    struct KeyColumn
    {
        QString key;
        QString column;
        KeyColumn() {}
        KeyColumn(const QString& k, const QString& c) : key(k), column(c) {}
    };

    QVector<KeyColumn> header_data;
    QVariantList model_data;
};

TBVariantListTableModel::TBVariantListTableModel(QObject* parent) : QAbstractTableModel(parent), pimpl(new impl)
{
}

TBVariantListTableModel::~TBVariantListTableModel() = default;

// set data
void TBVariantListTableModel::setListData(const QVariantList& list_data)
{
    this->beginResetModel();
    this->pimpl->model_data = list_data;
    this->endResetModel();
}

QVariantList TBVariantListTableModel::listData() const
{
    return this->pimpl->model_data;
}

void TBVariantListTableModel::addHeader(const QString& key, const QString& column)
{
    this->pimpl->header_data.push_back(TBVariantListTableModel::impl::KeyColumn(key, column));
}

QVariant TBVariantListTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if(this->pimpl->header_data.size() > section)
        {
            return this->pimpl->header_data[section].column;
        }
        else
        {
            qDebug() << "TBVariantListTableModel::headerData: section =" << section << ", headers size =" << this->pimpl->header_data.size();
        }
    }

    return QVariant();
}

int TBVariantListTableModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        return 0;
    }
    else
    {
        return this->pimpl->model_data.size();
    }
}

int TBVariantListTableModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        return 0;
    }
    else
    {
        return this->pimpl->header_data.size();
    }
}

QVariant TBVariantListTableModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
        // get model row
        if(this->pimpl->model_data.size() > index.row() && this->pimpl->header_data.size() > index.column())
        {
            auto row_data(this->pimpl->model_data[index.row()].toMap());
            auto key(this->pimpl->header_data[index.column()].key);
            return row_data[key];
        }

        qDebug() << "TBVariantListTableModel::data: index =" << index << ", headers size =" << this->pimpl->header_data.size() << ", data size =" << this->pimpl->model_data.size();
    }

    return QVariant();
}

bool TBVariantListTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    
    if (index.row() >= this->pimpl->model_data.size() || index.column() >= this->pimpl->header_data.size())
        return false;
        
    if (data(index, role) != value) {
        // Get the row data as a mutable map
        QVariantMap rowData = this->pimpl->model_data[index.row()].toMap();
        QString key = this->pimpl->header_data[index.column()].key;
        
        // Update the value
        rowData[key] = value;
        
        // Store the updated row back
        this->pimpl->model_data[index.row()] = rowData;
        
        Q_EMIT dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags TBVariantListTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool TBVariantListTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return false;
}

bool TBVariantListTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
    return false;
}

QVariantMap TBVariantListTableModel::getRowData(int row) const
{
    if (row >= 0 && row < this->pimpl->model_data.size())
    {
        return this->pimpl->model_data[row].toMap();
    }
    return QVariantMap();
}
