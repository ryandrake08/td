#pragma once

#include <QString>
#include <QVariantList>
#include <QWidget>
#include <memory>

class QStandardItemModel;
namespace Ui { class TBTableWidget; }

// Widget representing a single tournament table with seats and players
// Matches macOS TBSeatingChartCollectionViewItem design
class TBTableWidget : public QWidget
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

    // updaters for various changeable UI controls
    void updateSeatsTable();

public:
    explicit TBTableWidget(QWidget* parent = nullptr);
    virtual ~TBTableWidget() override;

    // Set table data
    void setTableName(const QString& tableName);
    void setSeats(const QVariantList& seats);

    // Get preferred size for this table
    QSize sizeHint() const override;
};