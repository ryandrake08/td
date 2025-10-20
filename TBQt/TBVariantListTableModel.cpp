#include "TBVariantListTableModel.hpp"

#include <QDebug>
#include <QList>
#include <QVariant>
#include <QVector>
#include <utility>

struct TBVariantListTableModel::impl
{
    struct KeyColumn
    {
        QString key;
        QString column;
        bool isIndexColumn;
        int indexOffset;
        KeyColumn() : isIndexColumn(false), indexOffset(1) {}
        KeyColumn(QString k, QString c) : key(std::move(k)), column(std::move(c)), isIndexColumn(false), indexOffset(1) {}
        KeyColumn(QString k, QString c, int offset) : key(std::move(k)), column(std::move(c)), isIndexColumn(true), indexOffset(offset) {}
    };

    QVector<KeyColumn> header_data;
    QVariantList model_data;
};

TBVariantListTableModel::TBVariantListTableModel(QObject* parent) : QAbstractTableModel(parent), pimpl(new impl())
{
}

TBVariantListTableModel::~TBVariantListTableModel() = default;

// set data
void TBVariantListTableModel::setListData(const QVariantList& data)
{
    this->beginResetModel();
    this->pimpl->model_data = data;
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

void TBVariantListTableModel::addIndexHeader(const QString& key, const QString& column, int offset)
{
    this->pimpl->header_data.push_back(TBVariantListTableModel::impl::KeyColumn(key, column, offset));
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

    return {};
}

int TBVariantListTableModel::rowCount(const QModelIndex& parent) const
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

int TBVariantListTableModel::columnCount(const QModelIndex& parent) const
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

QVariant TBVariantListTableModel::data(const QModelIndex& index, int role) const
{
    if(index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole))
    {
        // get model row
        if(this->pimpl->model_data.size() > index.row() && this->pimpl->header_data.size() > index.column())
        {
            const auto& columnInfo = this->pimpl->header_data[index.column()];

            if(columnInfo.isIndexColumn)
            {
                // Return computed index value
                return index.row() + columnInfo.indexOffset;
            }
            else
            {
                // Return data from model
                auto row_data(this->pimpl->model_data[index.row()].toMap());
                return row_data[columnInfo.key];
            }
        }

        qDebug() << "TBVariantListTableModel::data: index =" << index << ", headers size =" << this->pimpl->header_data.size() << ", data size =" << this->pimpl->model_data.size();
    }

    return {};
}

bool TBVariantListTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid() || role != Qt::EditRole)
        return false;

    if(index.row() >= this->pimpl->model_data.size() || index.column() >= this->pimpl->header_data.size())
        return false;

    const auto& columnInfo = this->pimpl->header_data[index.column()];

    // Index columns are not editable
    if(columnInfo.isIndexColumn)
        return false;

    if(data(index, role) != value)
    {
        // Get the row data as a mutable map
        QVariantMap rowData = this->pimpl->model_data[index.row()].toMap();

        // Update the value
        rowData[columnInfo.key] = value;

        // Store the updated row back
        this->pimpl->model_data[index.row()] = rowData;

        Q_EMIT dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags TBVariantListTableModel::flags(const QModelIndex& index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;

    if(index.column() < this->pimpl->header_data.size())
    {
        const auto& columnInfo = this->pimpl->header_data[index.column()];
        if(columnInfo.isIndexColumn)
        {
            // Index columns are not editable
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool TBVariantListTableModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if(count <= 0 || row < 0 || row > this->pimpl->model_data.size())
    {
        return false;
    }

    beginInsertRows(parent, row, row + count - 1);

    // Insert empty QVariantMap objects at the specified position
    for(int i = 0; i < count; ++i)
    {
        this->pimpl->model_data.insert(row, QVariantMap());
    }

    endInsertRows();
    return true;
}

bool TBVariantListTableModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if(count <= 0 || row < 0 || row + count > this->pimpl->model_data.size())
    {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);

    // Remove rows from the data list
    // Remove in reverse order to avoid index shifting issues
    for(int i = row + count - 1; i >= row; --i)
    {
        this->pimpl->model_data.removeAt(i);
    }

    endRemoveRows();
    return true;
}

QVariantMap TBVariantListTableModel::getRowData(int row) const
{
    if(row >= 0 && row < this->pimpl->model_data.size())
    {
        return this->pimpl->model_data[row].toMap();
    }
    return {};
}
