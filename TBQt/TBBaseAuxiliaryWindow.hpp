#pragma once

#include <QWidget>

class QCloseEvent;

// Base class for auxiliary tournament windows (seating chart, display, etc.)
// Provides common window setup, session handling, and lifecycle management
class TBBaseAuxiliaryWindow : public QWidget
{
    Q_OBJECT

protected:
    // Set background and text colors using Qt stylesheet and update backgroundIsDark property
    void setBackgroundColorString(const QString& backgroundColorString);

protected:
    // Window-specific icon override system for custom backgrounds - override in derived classes
    virtual void overrideIconsForBackground(bool isDark);
    virtual void restoreThemeBasedIcons();

public:
    explicit TBBaseAuxiliaryWindow(QWidget* parent = nullptr);
    virtual ~TBBaseAuxiliaryWindow() override;

    // Apply display settings (screen, fullscreen, etc.) for this window
    void showUsingDisplaySettings(const QString& windowType);
};