#pragma once

#include <QWidget>
#include <memory>

class TournamentSession;

// Standalone action clock window
class TBActionClockWindow : public QWidget
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

    // updaters for various changeable UI controls
    void updateActionClock(const QVariantMap& state);

private Q_SLOTS:
    void on_tournamentStateChanged(const QString& key, const QVariant& value);

protected:
    void closeEvent(QCloseEvent* event) override;

public:
    explicit TBActionClockWindow(TournamentSession& session, QWidget* parent = nullptr);
    virtual ~TBActionClockWindow();

    // Show and center the window over the parent with 3/4 size
    void showCenteredOverParent();

Q_SIGNALS:
    void clockCanceled();
};