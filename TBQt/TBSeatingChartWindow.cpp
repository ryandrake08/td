#include "TBSeatingChartWindow.hpp"
#include "TBTableWidget.hpp"
#include "TournamentSession.hpp"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QWidget>
#include <QVariantMap>
#include <QVariantList>
#include <QDebug>
#include <QApplication>

struct TBSeatingChartWindow::impl
{
    QScrollArea* scrollArea;
    QWidget* contentWidget;
    QVBoxLayout* contentLayout;
    QLabel* titleLabel;
    QMap<QString, QVariantList> tables; // Table name -> list of seats
    QList<TBTableWidget*> tableWidgets;

    impl() : scrollArea(nullptr), contentWidget(nullptr),
             contentLayout(nullptr), titleLabel(nullptr) {}
};

TBSeatingChartWindow::TBSeatingChartWindow(QWidget* parent)
    : TBBaseMainWindow(parent), pimpl(new impl)
{
    setupUI();

    // Connect to session state changes
    QObject::connect(&this->getSession(), &TournamentSession::stateChanged,
                    this, &TBSeatingChartWindow::on_tournamentStateChanged);

    // Initial update
    updateWindowTitle();
    updateSeatingChart();
    updateBackgroundColor();
}

TBSeatingChartWindow::~TBSeatingChartWindow() = default;

void TBSeatingChartWindow::on_authorizedChanged(bool auth)
{
    Q_UNUSED(auth)
    // Seating chart doesn't need to handle authorization changes differently
    // It's a display-only window that inherits session access from TBBaseMainWindow
}

void TBSeatingChartWindow::setupUI()
{
    setWindowTitle("Tournament Seating Chart");
    setMinimumSize(800, 600);
    resize(1000, 700);

    // Create central widget (QMainWindow needs a central widget, not a direct layout)
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main layout on the central widget
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setMargin(10);

    // Title label
    pimpl->titleLabel = new QLabel("Tournament Seating Chart", centralWidget);
    pimpl->titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = pimpl->titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    pimpl->titleLabel->setFont(titleFont);
    mainLayout->addWidget(pimpl->titleLabel);

    // Scroll area for tables
    pimpl->scrollArea = new QScrollArea(centralWidget);
    pimpl->scrollArea->setWidgetResizable(true);
    pimpl->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    pimpl->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Content widget inside scroll area
    pimpl->contentWidget = new QWidget();
    pimpl->contentLayout = new QVBoxLayout(pimpl->contentWidget);
    pimpl->contentLayout->setAlignment(Qt::AlignTop);
    pimpl->contentLayout->setSpacing(15);

    pimpl->scrollArea->setWidget(pimpl->contentWidget);
    mainLayout->addWidget(pimpl->scrollArea);
}


void TBSeatingChartWindow::on_tournamentStateChanged(const QString& key, const QVariant& value)
{
    Q_UNUSED(value)

    if (key == "seating_chart") {
        updateSeatingChart();
    } else if (key == "name") {
        updateWindowTitle();
    } else if (key == "background_color") {
        updateBackgroundColor();
    }
}

void TBSeatingChartWindow::updateWindowTitle()
{
    const QVariantMap& state = this->getSession().state();
    QString tournamentName = state.value("name").toString();

    if (tournamentName.isEmpty()) {
        setWindowTitle("Tournament Seating Chart");
        pimpl->titleLabel->setText("Tournament Seating Chart");
    } else {
        setWindowTitle(QString("Seating Chart: %1").arg(tournamentName));
        pimpl->titleLabel->setText(QString("Seating Chart: %1").arg(tournamentName));
    }
}

void TBSeatingChartWindow::updateBackgroundColor()
{
    const QVariantMap& state = this->getSession().state();
    QString backgroundColorName = state.value("background_color").toString();

    if (!backgroundColorName.isEmpty()) {
        // TODO: Implement background color support when CSS color parsing is added
        // For now, use default colors
        qDebug() << "Background color requested:" << backgroundColorName;
    }
}

void TBSeatingChartWindow::updateSeatingChart()
{
    const QVariantMap& state = this->getSession().state();
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
    // Clear existing table widgets
    for (TBTableWidget* widget : pimpl->tableWidgets) {
        pimpl->contentLayout->removeWidget(widget);
        widget->deleteLater();
    }
    pimpl->tableWidgets.clear();

    const QVariantMap& state = this->getSession().state();
    QVariantList tablesPlaying = state.value("tables_playing").toList();

    // Create table widgets for each active table
    for (const QVariant& tableNameVariant : tablesPlaying) {
        QString tableName = tableNameVariant.toString();

        if (!tableName.isEmpty()) {
            // Create table widget
            TBTableWidget* tableWidget = new TBTableWidget(pimpl->contentWidget);
            tableWidget->setTableName(tableName);

            // Set seats for this table
            QVariantList seats = pimpl->tables.value(tableName, QVariantList());
            tableWidget->setSeats(seats);

            // Add to layout and track
            pimpl->contentLayout->addWidget(tableWidget);
            pimpl->tableWidgets.append(tableWidget);
        }
    }

    // If no tables, show message
    if (pimpl->tableWidgets.isEmpty()) {
        QLabel* messageLabel = new QLabel("No active tables in tournament", pimpl->contentWidget);
        messageLabel->setAlignment(Qt::AlignCenter);
        messageLabel->setStyleSheet("color: gray; font-size: 14px; margin: 50px;");
        pimpl->contentLayout->addWidget(messageLabel);
    }

    // Update layout
    pimpl->contentWidget->adjustSize();
}