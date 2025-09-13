#pragma once

#include "TBBaseAuxiliaryWindow.hpp"
#include <memory>

class TournamentSession;

// Standalone action clock window
class TBActionClockWindow : public TBBaseAuxiliaryWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBActionClockWindow(const TournamentSession& session, QWidget* parent = nullptr);
    virtual ~TBActionClockWindow();

    // Show and center the window over the parent with 3/4 size
    void showCenteredOverParent();

    // updaters for various changeable UI controls
    void updateActionClock(const QVariantMap& state);
};