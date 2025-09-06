#include "TBActionClockWindow.hpp"

#include "TBActionClockWidget.hpp"

#include "TournamentSession.hpp"

#include <QCloseEvent>
#include <QVBoxLayout>

struct TBActionClockWindow::impl
{
    TournamentSession& session;

    // Child widgets
    TBActionClockWidget* clockWidget;

    explicit impl(TournamentSession& sess, TBActionClockWindow* parent) : session(sess), clockWidget(new TBActionClockWidget(parent)) {}
};

TBActionClockWindow::TBActionClockWindow(TournamentSession& session, QWidget* parent) : QWidget(parent), pimpl(new impl(session, this))
{
    // Set window properties
    setWindowTitle(tr("Action Clock"));
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // Create layout and add clock widget
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(pimpl->clockWidget);

    // Connect to session state changes
    QObject::connect(&pimpl->session, &TournamentSession::stateChanged, this, &TBActionClockWindow::on_tournamentStateChanged);

    // Initialize clock state
    this->updateActionClock();
}

TBActionClockWindow::~TBActionClockWindow() = default;

void TBActionClockWindow::showCenteredOverParent()
{
    if(!parentWidget())
    {
        show();
        return;
    }

    // Calculate size as 3/4 of parent's smaller dimension
    QSize parentSize = parentWidget()->size();
    int minDimension = qMin(parentSize.width(), parentSize.height());
    int clockSize = (minDimension * 3) / 4;

    // Set square size
    resize(clockSize, clockSize);

    // Center over parent
    QPoint parentCenter = parentWidget()->geometry().center();
    QPoint topLeft = parentCenter - QPoint(clockSize / 2, clockSize / 2);
    move(topLeft);

    show();
    raise();
    activateWindow();
}

void TBActionClockWindow::on_tournamentStateChanged(const QString& key, const QVariant& value)
{
    Q_UNUSED(value);

    if(key == "action_clock_time_remaining")
    {
        this->updateActionClock();
    }
}

void TBActionClockWindow::closeEvent(QCloseEvent* event)
{
    // Closing the window cancels the clock
    Q_EMIT clockCanceled();
    event->accept();
}

void TBActionClockWindow::updateActionClock()
{
    const QVariantMap& state = pimpl->session.state();

    int timeRemaining = state["action_clock_time_remaining"].toInt();
    if(timeRemaining > 0)
    {
        // Clock is active - update display and show window
        double seconds = timeRemaining / 1000.0;
        pimpl->clockWidget->setTimeRemaining(seconds);

        // Show window if not already visible
        if(!isVisible())
        {
            showCenteredOverParent();
        }
    }
    else
    {
        // Clock is not active - hide window
        if(isVisible())
        {
            hide();
        }
    }
}