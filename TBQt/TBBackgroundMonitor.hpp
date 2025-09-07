#pragma once

#include <QObject>
#include <memory>

class TBInvertableButton;

/**
 * Background monitor that automatically detects when a widget's background changes
 * from light to dark (or vice versa) and notifies registered invertable buttons.
 *
 * Uses event-driven detection (QEvent::ApplicationPaletteChange, QEvent::ThemeChange)
 * instead of polling to efficiently respond to system theme changes.
 *
 * This provides automatic dark mode adaptation without requiring manual coordination
 * between parent widgets and their invertable buttons.
 */
class TBBackgroundMonitor : public QObject
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBBackgroundMonitor(QWidget* parent);
    virtual ~TBBackgroundMonitor();

    // Register/unregister buttons to be automatically updated
    void registerButton(TBInvertableButton* button);
    void unregisterButton(TBInvertableButton* button);

    // Manually check and update the state of registered buttons
    void checkAndUpdateButtons();

Q_SIGNALS:
    // Emitted when background darkness changes
    void backgroundDarknessChanged(bool isDark);

protected:
    // Override to catch palette/theme change events
    bool eventFilter(QObject* watched, QEvent* event) override;

private Q_SLOTS:
    void onWatchedWidgetDestroyed();
};