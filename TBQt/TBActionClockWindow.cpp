#include "TBActionClockWindow.hpp"

#include "TBActionClockWidget.hpp"

#include "TournamentSession.hpp"

#include <QCloseEvent>
#include <QVBoxLayout>

struct TBActionClockWindow::impl
{
    // Child widgets
    TBActionClockWidget* clockWidget;

    explicit impl(TBActionClockWindow* parent) : clockWidget(new TBActionClockWidget(parent)) {}
};

TBActionClockWindow::TBActionClockWindow(const TournamentSession& session, QWidget* parent) : TBBaseAuxiliaryWindow(parent), pimpl(new impl(this))
{
    // Set window title
    setWindowTitle(QObject::tr("Action Clock"));

    // Create layout and add clock widget
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(pimpl->clockWidget);

    // Initialize clock state
    this->updateActionClock(session.state());
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

void TBActionClockWindow::updateActionClock(const QVariantMap& state)
{
    int timeRemaining = state["action_clock_time_remaining"].toInt();
    if(timeRemaining > 0)
    {
        // Clock is active - update display and show window
        double seconds = timeRemaining / 1000.0;
        pimpl->clockWidget->setTimeRemaining(seconds);
    }
}