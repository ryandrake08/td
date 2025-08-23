#pragma once

#include <QWidget>
#include <memory>

class TBActionClockWidget : public QWidget
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBActionClockWidget(QWidget* parent = nullptr);
    ~TBActionClockWidget() override;

    // Clock state management - driven by tournament daemon
    void setTimeRemaining(double seconds);
    void show();
    void hide();

    // Configuration properties
    void setEnableShadows(bool enable);
    void setEnableGraduations(bool enable);
    void setEnableDigit(bool enable);
    void setEnableArc(bool enable);
    void setArcFillsIn(bool fillsIn);

    // Size hint for proper overlay positioning
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};