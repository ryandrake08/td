#include "TBSeatingChartWindow.hpp"
#include "ui_TBSeatingChartWindow.h"
#include "TBTableWidget.hpp"
#include "TBFlowLayout.hpp"
#include "TournamentSession.hpp"

#include <QLabel>
#include <QVariantMap>
#include <QVariantList>

struct TBSeatingChartWindow::impl
{
    // UI
    std::unique_ptr<Ui::TBSeatingChartWindow> ui;

    // Session reference
    TournamentSession& session;

    // Internal data
    QMap<QString, QVariantList> tables; // Table name -> list of seats
    QList<TBTableWidget*> tableWidgets;

    impl(TournamentSession& sess) : ui(new Ui::TBSeatingChartWindow), session(sess) {}
};

TBSeatingChartWindow::TBSeatingChartWindow(TournamentSession& tournamentSession, QWidget* parent)
    : QMainWindow(parent), pimpl(new impl(tournamentSession))
{
    pimpl->ui->setupUi(this);

    // Set up flow layout for the table grid
    auto* flowLayout = new TBFlowLayout(15, 15); // 15px spacing horizontal and vertical
    pimpl->ui->scrollAreaWidgetContents->setLayout(flowLayout);

    // Connect to session state changes
    QObject::connect(&pimpl->session, &TournamentSession::stateChanged,
                    this, &TBSeatingChartWindow::on_tournamentStateChanged);

    // Initial update
    updateWindowTitle();
    updateTournamentInfo();
    updateSeatingChart();
    updateBackgroundColor();
}

TBSeatingChartWindow::~TBSeatingChartWindow() = default;

void TBSeatingChartWindow::on_tournamentStateChanged(const QString& key, const QVariant& value)
{
    Q_UNUSED(value)

    if (key == "seating_chart") {
        updateSeatingChart();
    } else if (key == "name") {
        updateWindowTitle();
        updateTournamentInfo();
    } else if (key == "buyin_text") {
        updateTournamentInfo();
    } else if (key == "background_color") {
        updateBackgroundColor();
    }
}

void TBSeatingChartWindow::updateWindowTitle()
{
    const QVariantMap& state = pimpl->session.state();
    QString tournamentName = state.value("name").toString();

    if (tournamentName.isEmpty()) {
        setWindowTitle("Tournament Seating Chart");
    } else {
        setWindowTitle(QString("Seating Chart: %1").arg(tournamentName));
    }
}

void TBSeatingChartWindow::updateTournamentInfo()
{
    const QVariantMap& state = pimpl->session.state();

    // Tournament name
    QString tournamentName = state.value("name", "Tournament").toString();
    pimpl->ui->tournamentNameLabel->setText(tournamentName);

    // Buyin information - use formatted buyin_text from derived state
    QString buyinText = state.value("buyin_text").toString();
    if (buyinText.isEmpty()) {
        pimpl->ui->buyinLabel->setText("Tournament Information");
    } else {
        pimpl->ui->buyinLabel->setText(buyinText);
    }
}

void TBSeatingChartWindow::updateBackgroundColor()
{
    const QVariantMap& state = pimpl->session.state();
    QString backgroundColorName = state.value("background_color").toString();

    if (!backgroundColorName.isEmpty()) {
        // TODO: Implement background color support when CSS color parsing is added
        // For now, use default colors
    }
}

void TBSeatingChartWindow::updateSeatingChart()
{
    const QVariantMap& state = pimpl->session.state();
    QVariantList seatingChart = state.value("seating_chart").toList();

    // Clear existing tables data
    pimpl->tables.clear();

    // Build tables dictionary from seating chart
    for (const QVariant& entryVariant : seatingChart) {
        QVariantMap entry = entryVariant.toMap();
        QString tableName = entry.value("table_name").toString();

        if (!tableName.isEmpty()) {
            // Add seat to table's seat list
            if (!pimpl->tables.contains(tableName)) {
                pimpl->tables[tableName] = QVariantList();
            }
            pimpl->tables[tableName].append(entry);
        }
    }

    // Rebuild table widgets
    rebuildTableWidgets();
}

void TBSeatingChartWindow::rebuildTableWidgets()
{
    // Get the flow layout
    TBFlowLayout* flowLayout = static_cast<TBFlowLayout*>(pimpl->ui->scrollAreaWidgetContents->layout());
    if (!flowLayout) {
        return;
    }

    // Clear ALL existing widgets from the layout (tables and message labels)
    QLayoutItem* item;
    while ((item = flowLayout->takeAt(0)) != nullptr) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
    pimpl->tableWidgets.clear();

    const QVariantMap& state = pimpl->session.state();
    QVariantList tablesPlaying = state.value("tables_playing").toList();

    // Create table widgets for each active table
    for (const QVariant& tableNameVariant : tablesPlaying) {
        QString tableName = tableNameVariant.toString();

        if (!tableName.isEmpty()) {
            // Create table widget
            TBTableWidget* tableWidget = new TBTableWidget(pimpl->ui->scrollAreaWidgetContents);
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
    if (pimpl->tableWidgets.isEmpty()) {
        QLabel* messageLabel = new QLabel("No active tables in tournament");
        messageLabel->setAlignment(Qt::AlignCenter);
        messageLabel->setStyleSheet("color: gray; font-size: 14px; margin: 50px;");
        messageLabel->setMinimumSize(400, 100);
        flowLayout->addWidget(messageLabel);
    }

    // Force layout update
    pimpl->ui->scrollAreaWidgetContents->updateGeometry();
    flowLayout->update();
}