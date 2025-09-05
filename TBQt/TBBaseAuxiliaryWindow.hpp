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

public:
    explicit TBBaseAuxiliaryWindow(QWidget* parent = nullptr);
    virtual ~TBBaseAuxiliaryWindow() override;

Q_SIGNALS:
    void windowClosed();
};