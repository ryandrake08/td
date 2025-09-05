#pragma once

#include <QWidget>

class QCloseEvent;

// Base class for auxiliary tournament windows (seating chart, display, etc.)
// Provides common window setup, session handling, and lifecycle management
class TBBaseAuxiliaryWindow : public QWidget
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent* event) override;

    // Set background and text colors using Qt stylesheet and update backgroundIsDark property
    void setBackgroundColorString(const QString& backgroundColorString);

    // Query current widget background color to determine if it's dark
    bool isBackgroundDark() const;

public:
    explicit TBBaseAuxiliaryWindow(QWidget* parent = nullptr);
    virtual ~TBBaseAuxiliaryWindow() override;

Q_SIGNALS:
    void backgroundIsDarkChanged(bool isDark);
    void windowClosed();
};