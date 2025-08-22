#pragma once

#include <QWidget>
#include <QVariantList>
#include <QString>
#include <memory>

// Widget representing a single tournament table with seats and players
class TBTableWidget : public QWidget
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBTableWidget(QWidget* parent = nullptr);
    virtual ~TBTableWidget() override;

    // Set table data
    void setTableName(const QString& tableName);
    void setSeats(const QVariantList& seats);

    // Get preferred size for this table
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void updateLayout();
};