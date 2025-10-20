#include "TBBaseMainWindow.hpp"

#include "TBActionClockWindow.hpp"
#include "TBFlowLayout.hpp"
#include "TBRuntimeError.hpp"
#include "TBSeatingChartWindow.hpp"
#include "TBSettingsDialog.hpp"
#include "TBSoundPlayer.hpp"
#include "TBTableWidget.hpp"
#include "TBTournamentDisplayWindow.hpp"

#include "TournamentSession.hpp"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QIcon>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QPrintDialog>
#include <QPrinter>

struct TBBaseMainWindow::impl
{
    // tournament session
    TournamentSession session;

    // common child windows
    TBSeatingChartWindow* seatingChartWindow { nullptr };
    TBTournamentDisplayWindow* displayWindow { nullptr };
    TBActionClockWindow* actionClockWindow { nullptr };

    // sound notifications
    TBSoundPlayer* soundPlayer;

    explicit impl(TBBaseMainWindow* parent) : soundPlayer(new TBSoundPlayer(session, parent)) {}
};

TBBaseMainWindow::TBBaseMainWindow(QWidget* parent) : QMainWindow(parent), pimpl(new impl(this))
{
    // set up rest of window
    this->setUnifiedTitleAndToolBarOnMac(true);

    // Set initial theme based on current system palette
    QColor windowColor = QApplication::palette().color(QPalette::Window);
    QIcon::setThemeName(windowColor.lightnessF() < 0.5 ? "dark_theme" : "light_theme");
}

TBBaseMainWindow::~TBBaseMainWindow() = default;

// Events

void TBBaseMainWindow::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange)
    {
        QColor windowColor = QApplication::palette().color(QPalette::Window);
        QIcon::setThemeName(windowColor.lightnessF() < 0.5 ? "dark_theme" : "light_theme");
    }

    QMainWindow::changeEvent(event);
}

void TBBaseMainWindow::closeEvent(QCloseEvent* /* event */)
{
    // disconnect from session
    pimpl->session.disconnect();
}

TournamentSession& TBBaseMainWindow::getSession()
{
    return pimpl->session;
}

bool TBBaseMainWindow::isSeatingChartWindowVisible() const
{
    return pimpl->seatingChartWindow && pimpl->seatingChartWindow->isVisible();
}

bool TBBaseMainWindow::isDisplayWindowVisible() const
{
    return pimpl->displayWindow && pimpl->displayWindow->isVisible();
}

void TBBaseMainWindow::on_actionPauseResume_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.toggle_pause_game();
    }
    else
    {
        pimpl->session.start_game();
    }
}

void TBBaseMainWindow::on_actionPreviousRound_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.set_previous_level();
    }
}

void TBBaseMainWindow::on_actionNextRound_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.set_next_level();
    }
}

void TBBaseMainWindow::on_actionCallClock_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        const auto& actionClockTimeRemaining(pimpl->session.state()["action_clock_time_remaining"].toInt());
        if(actionClockTimeRemaining == 0)
        {
            pimpl->session.set_action_clock(TournamentSession::kActionClockRequestTime);
        }
        else
        {
            pimpl->session.clear_action_clock();
        }
    }
}

void TBBaseMainWindow::on_actionCancelClock_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.clear_action_clock();
    }
}

void TBBaseMainWindow::on_actionEndGame_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.stop_game();
    }
}

void TBBaseMainWindow::on_actionShowHideSeatingChart_triggered()
{
    if(pimpl->seatingChartWindow && pimpl->seatingChartWindow->isVisible())
    {
        // Close and destroy the window
        pimpl->seatingChartWindow->close();
    }
    else
    {
        // Create new window if it doesn't exist
        if(!pimpl->seatingChartWindow)
        {
            pimpl->seatingChartWindow = new TBSeatingChartWindow(pimpl->session, this);

            // Connect to destroyed signal to handle automatic destruction by closing
            QObject::connect(pimpl->seatingChartWindow, &QObject::destroyed, this, [this]()
            {
                pimpl->seatingChartWindow = nullptr;
                this->updateSeatingChartMenuText();
            });
        }

        // Apply settings and show
        pimpl->seatingChartWindow->showUsingDisplaySettings("SeatingChart");
    }
    this->updateSeatingChartMenuText();
}

void TBBaseMainWindow::on_actionShowHideMainDisplay_triggered()
{
    if(pimpl->displayWindow && pimpl->displayWindow->isVisible())
    {
        // Close and destroy the window
        pimpl->displayWindow->close();
    }
    else
    {
        // Create new window if it doesn't exist
        if(!pimpl->displayWindow)
        {
            pimpl->displayWindow = new TBTournamentDisplayWindow(pimpl->session, this);

            // Connect to destroyed signal to handle automatic destruction by closing
            QObject::connect(pimpl->displayWindow, &QObject::destroyed, this, [this]()
            {
                pimpl->displayWindow = nullptr;
                this->updateDisplayMenuText();
            });

            // Connect TBTournamentDisplayWindow action signals to base main window slots
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::previousRoundRequested, this, &TBBaseMainWindow::on_actionPreviousRound_triggered);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::pauseResumeRequested, this, &TBBaseMainWindow::on_actionPauseResume_triggered);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::nextRoundRequested, this, &TBBaseMainWindow::on_actionNextRound_triggered);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::callClockRequested, this, &TBBaseMainWindow::on_actionCallClock_triggered);
        }

        // Apply settings and show
        pimpl->displayWindow->showUsingDisplaySettings("TournamentDisplay");
    }
    this->updateDisplayMenuText();
}

void TBBaseMainWindow::on_actionMinimize_triggered()
{
    this->showMinimized();
}

void TBBaseMainWindow::on_actionZoom_triggered()
{
    if(this->isMaximized())
    {
        this->showNormal();
    }
    else
    {
        this->showMaximized();
    }
}

void TBBaseMainWindow::on_actionBringAllToFront_triggered()
{
    this->raise();
    this->activateWindow();
}

void TBBaseMainWindow::on_actionHelp_triggered()
{
    QMessageBox message(this);
    message.setIcon(QMessageBox::Information);
    message.setIconPixmap(QIcon::fromTheme("i_application").pixmap(64, 64));
    message.setWindowTitle(QObject::tr("Help"));
    message.setText(QObject::tr("Help isn't available for %1").arg(QCoreApplication::applicationName()));
    message.exec();
}

void TBBaseMainWindow::on_actionAbout_triggered()
{
    // show about box
    QMessageBox message(this);
    message.setIconPixmap(QIcon::fromTheme("i_application").pixmap(64, 64));
    message.setWindowTitle(QObject::tr("About %1...").arg(QCoreApplication::applicationName()));
    message.setText(QCoreApplication::applicationName());
    message.setInformativeText(QObject::tr("Version %1").arg(QCoreApplication::applicationVersion()));
    message.exec();
}

void TBBaseMainWindow::on_actionSettings_triggered()
{
    TBSettingsDialog dialog(this);
    dialog.exec();
}

void TBBaseMainWindow::on_actionExit_triggered()
{
    this->close();
}

void TBBaseMainWindow::updateDisplayMenuText()
{
    // Update menu text based on display window visibility
    bool isVisible = this->isDisplayWindowVisible();
    QString menuText = isVisible ? QObject::tr("Hide Main Display") : QObject::tr("Show Main Display");

    // Find the action by name (both derived classes use the same name)
    auto* action = this->findChild<QAction*>("actionShowHideMainDisplay");
    if(action)
    {
        action->setText(menuText);
    }
}

void TBBaseMainWindow::updateSeatingChartMenuText()
{
    // Update menu text based on seating chart window visibility
    bool isVisible = this->isSeatingChartWindowVisible();
    QString menuText = isVisible ? QObject::tr("Hide Seating Chart") : QObject::tr("Show Seating Chart");

    // Find the action by name (both derived classes use the same name)
    auto* action = this->findChild<QAction*>("actionShowHideSeatingChart");
    if(action)
    {
        action->setText(menuText);
    }
}

void TBBaseMainWindow::updateActionClock(const QVariantMap& state)
{

    int timeRemaining = state["action_clock_time_remaining"].toInt();
    if(timeRemaining == 0)
    {
        // Clock is not active - hide window
        if(pimpl->actionClockWindow)
        {
            pimpl->actionClockWindow->close();
        }
    }
    else
    {
        // If the window is not yet created
        if(!pimpl->actionClockWindow)
        {
            QWidget* parent(this);
            if(pimpl->displayWindow)
            {
                parent = pimpl->displayWindow;
            }

            // Create the window, parent is either the display (if open) or this window
            pimpl->actionClockWindow = new TBActionClockWindow(pimpl->session, parent);

            // Connect to destroyed signal to handle automatic destruction by closing
            QObject::connect(pimpl->actionClockWindow, &QObject::destroyed, this, [this]()
            {
                pimpl->actionClockWindow = nullptr;
                this->on_actionCancelClock_triggered();
            });
        }

        // Show window if not already visible
        if(!pimpl->actionClockWindow->isVisible())
        {
            pimpl->actionClockWindow->showCenteredOverParent();
        }

        // Update the clock itself
        pimpl->actionClockWindow->updateActionClock(state);
    }
}

void TBBaseMainWindow::on_actionExport_triggered()
{
    QFileDialog picker(this);
    picker.setAcceptMode(QFileDialog::AcceptSave);
    picker.setFileMode(QFileDialog::AnyFile);
    picker.setNameFilter(QObject::tr("CSV Files (*.csv)"));
    picker.setViewMode(QFileDialog::Detail);
    if(picker.exec())
    {
        for(const auto& filename : picker.selectedFiles())
        {
            // get results
            auto results(this->getSession().results_as_csv());

            // create and open file
            QFile file_obj(filename);
            if(!file_obj.open(QFile::WriteOnly | QFile::Text))
            {
                // handle file open failure
                throw TBRuntimeError(QObject::tr("Cannot write file %1:\n%2.").arg(QDir::toNativeSeparators(filename), file_obj.errorString()));
            }

            // write to file
            file_obj.write(results);

            // close file
            file_obj.close();
        }
    }
}

void TBBaseMainWindow::on_actionPrintSeatingChart_triggered()
{
    // Get current tournament state
    QVariantMap state = this->pimpl->session.state();

    // Get seating chart data from state
    QVariantList seatingChart = state.value("seating_chart").toList();
    QVariantList tablesPlaying = state.value("tables_playing").toList();
    QString tournamentName = state.value("name").toString();
    QString buyinText = state.value("buyin_text").toString();

    // Check if there's anything to print
    if(tablesPlaying.isEmpty())
    {
        QMessageBox::information(this, QObject::tr("Print Seating Chart"),
                                 QObject::tr("No active tables to print."));
        return;
    }

    // Build tables dictionary from seating chart
    QMap<QString, QVariantList> tables;
    for(const QVariant& entryVariant : seatingChart)
    {
        QVariantMap entry = entryVariant.toMap();
        QString tableName = entry.value("table_name").toString();

        if(!tableName.isEmpty())
        {
            if(!tables.contains(tableName))
            {
                tables[tableName] = QVariantList();
            }
            tables[tableName].append(entry);
        }
    }

    // Create printer and show print dialog
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageOrientation(QPageLayout::Landscape);

    QPrintDialog printDialog(&printer, this);
    printDialog.setWindowTitle(QObject::tr("Print Seating Chart"));

    if(printDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    // Create painter for printing
    QPainter painter;
    if(!painter.begin(&printer))
    {
        QMessageBox::warning(this, QObject::tr("Print Seating Chart"),
                             QObject::tr("Failed to initialize printer."));
        return;
    }

    // Set up fonts and metrics - scale font sizes for printer DPI
    QFont headerFont = painter.font();
    headerFont.setPointSize(16);
    headerFont.setBold(true);

    QFont normalFont = painter.font();
    normalFont.setPointSize(10);

    qDebug() << "Header font point size:" << headerFont.pointSize();
    qDebug() << "Normal font point size:" << normalFont.pointSize();

    // Calculate page dimensions in device coordinates for proper scaling
    // Use device pixels for precise positioning, but account for resolution
    QRect pageRect = printer.pageRect(QPrinter::DevicePixel).toRect();

    // Get printer resolution to convert between screen and printer coordinates
    int printerDpiX = printer.resolution();

    // Typical screen DPI (Qt default is 96 on most platforms)
    const int screenDpi = 96;
    double dpiScale = static_cast<double>(printerDpiX) / screenDpi;

    int margin = static_cast<int>(50 * dpiScale);
    int contentWidth = pageRect.width() - 2 * margin;
    int yPos = margin;

    // Draw header - scale dimensions to printer DPI
    int headerHeight = static_cast<int>(40 * dpiScale);
    int headerSpacing = static_cast<int>(50 * dpiScale);
    int buyinHeight = static_cast<int>(30 * dpiScale);
    int buyinSpacing = static_cast<int>(40 * dpiScale);

    painter.setFont(headerFont);
    painter.drawText(margin, yPos, contentWidth, headerHeight, Qt::AlignCenter,
                     tournamentName.isEmpty() ? QObject::tr("Tournament Seating Chart") : tournamentName);
    yPos += headerSpacing;

    // Draw buy-in info if available
    if(!buyinText.isEmpty())
    {
        painter.setFont(normalFont);
        painter.drawText(margin, yPos, contentWidth, buyinHeight, Qt::AlignCenter, buyinText);
        yPos += buyinSpacing;
    }

    // Calculate table layout
    const int tableSpacing = 5; // Reduced to allow more tables per page
    const int tablesPerRow = 3; // Print 3 tables per row

    // Build list of table names to print
    QStringList tableNames;
    for(const QVariant& tableNameVariant : tablesPlaying)
    {
        QString tableName = tableNameVariant.toString();
        if(!tableName.isEmpty())
        {
            tableNames.append(tableName);
        }
    }

    if(tableNames.isEmpty())
    {
        painter.end();
        return;
    }

    // Create ONE reusable widget for rendering all tables
    // This is much faster than creating 63+ widget hierarchies
    auto* tableWidget = new TBTableWidget(nullptr);

    // Initialize with first table to get size
    tableWidget->setTableName(tableNames[0]);
    tableWidget->setSeats(tables.value(tableNames[0], QVariantList()));
    QSize sampleTableSize = tableWidget->sizeHint();

    // Convert table spacing to printer coordinates
    int tableSpacingPrinter = static_cast<int>(tableSpacing * dpiScale);

    // Calculate width available for each table in printer coordinates
    int tableWidth = contentWidth / tablesPerRow - tableSpacingPrinter;

    // Calculate scale factor: must account for DPI difference
    // Widget is in screen pixels, we need to scale it to printer pixels
    double scale = static_cast<double>(tableWidth) / (sampleTableSize.width() * dpiScale);

    // Calculate available height per row (target 2 rows in landscape)
    int availableHeightForTables = pageRect.height() - yPos - margin;
    int targetRowsPerPage = 2; // Aim for 2 rows in landscape
    int maxTableHeight = (availableHeightForTables / targetRowsPerPage) - tableSpacingPrinter;

    // Apply the scale to get height, then convert to printer pixels
    int scaledTableHeight = static_cast<int>(sampleTableSize.height() * scale * dpiScale);

    // Constrain to max height to ensure we can fit target rows
    scaledTableHeight = std::min(scaledTableHeight, maxTableHeight);

    // Set a minimum table height to ensure readability (convert to printer pixels)
    const int minTableHeight = static_cast<int>(100 * dpiScale);
    scaledTableHeight = std::max(scaledTableHeight, minTableHeight);

    // Recalculate scale based on final height to maintain aspect ratio
    scale = static_cast<double>(scaledTableHeight) / (sampleTableSize.height() * dpiScale);

    int rowHeight = scaledTableHeight + tableSpacingPrinter;

    // Calculate how many rows fit on a page (after the header)
    int availableHeight = pageRect.height() - yPos - margin;
    int rowsPerPage = std::max(1, availableHeight / rowHeight);
    int tablesPerPage = rowsPerPage * tablesPerRow;

    // Render tables in grid layout with proper pagination
    int tableIndex = 0;
    int currentYPos = yPos; // Start after header on first page

    while(tableIndex < tableNames.size())
    {
        // Render up to tablesPerPage tables on this page
        int tablesOnThisPage = std::min(tablesPerPage, tableNames.size() - tableIndex);

        for(int i = 0; i < tablesOnThisPage; ++i)
        {
            int row = i / tablesPerRow;
            int col = i % tablesPerRow;

            // Update the reusable widget with current table's data
            QString tableName = tableNames[tableIndex + i];
            tableWidget->setTableName(tableName);
            tableWidget->setSeats(tables.value(tableName, QVariantList()));

            // Calculate position
            int xPos = margin + col * (tableWidth + tableSpacingPrinter);
            int tableYPos = currentYPos + row * rowHeight;

            // Render the table widget
            painter.save();
            painter.translate(xPos, tableYPos);

            // Apply scaling: DPI conversion * fit-to-width scaling
            double combinedScale = scale * dpiScale;
            painter.scale(combinedScale, combinedScale);

            tableWidget->render(&painter);
            painter.restore();
        }

        tableIndex += tablesOnThisPage;

        // Start new page if there are more tables to render
        if(tableIndex < tableNames.size())
        {
            printer.newPage();
            currentYPos = margin;
        }
    }

    // Clean up the single reusable widget
    delete tableWidget;

    painter.end();
}
