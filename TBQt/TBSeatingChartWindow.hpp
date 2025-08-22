#pragma once

#include "TBBaseMainWindow.hpp"
#include <QVariantMap>
#include <memory>

// Tournament seating chart display window
class TBSeatingChartWindow : public TBBaseMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBSeatingChartWindow(QWidget* parent = nullptr);
    virtual ~TBSeatingChartWindow() override;

private Q_SLOTS:
    void on_authorizedChanged(bool auth) override;
    void on_tournamentStateChanged(const QString& key, const QVariant& value);
    void updateSeatingChart();
    void updateWindowTitle();
    void updateBackgroundColor();

private:
    void setupUI();
    void rebuildTableWidgets();
};