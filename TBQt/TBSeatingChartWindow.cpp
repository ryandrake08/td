#include "TBSeatingChartWindow.hpp"

#include "TBFlowLayout.hpp"
#include "TBTableWidget.hpp"

#include "TournamentSession.hpp"

#include "ui_TBSeatingChartWindow.h"

#include <QLabel>
#include <QVariantList>
#include <QVariantMap>

struct TBSeatingChartWindow::impl
{
    // UI
    Ui::TBSeatingChartWindow ui;

    // Session reference
    TournamentSession& session;

    // Internal data
    QMap<QString, QVariantList> tables; // Table name -> list of seats
    QList<TBTableWidget*> tableWidgets;

    explicit impl(TournamentSession& sess) : session(sess) {}
};

TBSeatingChartWindow::TBSeatingChartWindow(TournamentSession& sess, QWidget* parent) : TBBaseAuxiliaryWindow(parent), pimpl(new impl(sess))
{
    pimpl->ui.setupUi(this);

    // Set up flow layout for the table grid
    auto* flowLayout = new TBFlowLayout(15, 15); // 15px spacing horizontal and vertical
    pimpl->ui.scrollAreaWidgetContents->setLayout(flowLayout);

    // Connect to session state changes
    QObject::connect(&pimpl->session, &TournamentSession::stateChanged, this, &TBSeatingChartWindow::on_tournamentStateChanged);

    // Initial update
    this->updateTournamentName();
    this->updateTournamentBuyin();
    this->updateBackgroundColor();
    this->updateSeatingChart();
}

TBSeatingChartWindow::~TBSeatingChartWindow() = default;

void TBSeatingChartWindow::on_tournamentStateChanged(const QString& key, const QVariant& value)
{
    Q_UNUSED(value)

    if(key == "name")
    {
        this->updateTournamentName();
    }
    else if(key == "buyin_text")
    {
        this->updateTournamentBuyin();
    }
    else if(key == "background_color")
    {
        this->updateBackgroundColor();
    }
    else if(key == "seating_chart")
    {
        this->updateSeatingChart();
    }
}

void TBSeatingChartWindow::updateTournamentName()
{
    const QVariantMap& state = pimpl->session.state();
    QString tournamentName = state.value("name").toString();
    pimpl->ui.tournamentNameLabel->setText(tournamentName);

    // Window title
    if(tournamentName.isEmpty())
    {
        setWindowTitle(QObject::tr("Tournament Seating Chart"));
    }
    else
    {
        setWindowTitle(QString(QObject::tr("Tournament Seating Chart: %1")).arg(tournamentName));
    }
}

void TBSeatingChartWindow::updateTournamentBuyin()
{
    const QVariantMap& state = pimpl->session.state();
    pimpl->ui.buyinLabel->setText(state.value("buyin_text").toString());
}

void TBSeatingChartWindow::updateBackgroundColor()
{
    const QVariantMap& state = pimpl->session.state();

    // Background color
    QString backgroundColorName = state.value("background_color").toString();
    if(!backgroundColorName.isEmpty())
    {
        // TODO: Implement background color support when CSS color parsing is added
        // For now, use default colors
    }
}

void TBSeatingChartWindow::updateSeatingChart()
{
    const QVariantMap& state = pimpl->session.state();

    // Clear existing tables data
    pimpl->tables.clear();

    // Build tables dictionary from seating chart
    QVariantList seatingChart = state.value("seating_chart").toList();
    for(const QVariant& entryVariant : seatingChart)
    {
        QVariantMap entry = entryVariant.toMap();
        QString tableName = entry.value("table_name").toString();

        if(!tableName.isEmpty())
        {
            // Add seat to table's seat list
            if(!pimpl->tables.contains(tableName))
            {
                pimpl->tables[tableName] = QVariantList();
            }
            pimpl->tables[tableName].append(entry);
        }
    }

    // Get the flow layout
    auto flowLayout = static_cast<TBFlowLayout*>(pimpl->ui.scrollAreaWidgetContents->layout());
    if(!flowLayout)
    {
        return;
    }

    // Clear ALL existing widgets from the layout (tables and message labels)
    QLayoutItem* item;
    while((item = flowLayout->takeAt(0)) != nullptr)
    {
        if(QWidget* widget = item->widget())
        {
            widget->deleteLater();
        }
        delete item;
    }
    pimpl->tableWidgets.clear();

    // Create table widgets for each active table
    QVariantList tablesPlaying = state.value("tables_playing").toList();
    for(const QVariant& tableNameVariant : tablesPlaying)
    {
        QString tableName = tableNameVariant.toString();

        if(!tableName.isEmpty())
        {
            // Create table widget
            TBTableWidget* tableWidget = new TBTableWidget(pimpl->ui.scrollAreaWidgetContents);
            tableWidget->setTableName(tableName);

            // Set seats for this table
            QVariantList seats = pimpl->tables.value(tableName, QVariantList());
            tableWidget->setSeats(seats);

            // Add to flow layout and track
            flowLayout->addWidget(tableWidget);
            pimpl->tableWidgets.append(tableWidget);
        }
    }

    // If no tables, show message
    if(pimpl->tableWidgets.isEmpty())
    {
        QLabel* messageLabel = new QLabel("No active tables in tournament");
        messageLabel->setAlignment(Qt::AlignCenter);
        messageLabel->setStyleSheet("color: gray; font-size: 14px; margin: 50px;");
        messageLabel->setMinimumSize(400, 100);
        flowLayout->addWidget(messageLabel);
    }

    // Force layout update
    pimpl->ui.scrollAreaWidgetContents->updateGeometry();
    flowLayout->update();
}