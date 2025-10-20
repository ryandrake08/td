#pragma once

#include <QDialog>
#include <memory>

class TBGenerateRoundsDialog : public QDialog
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBGenerateRoundsDialog(QWidget* parent = nullptr);
    ~TBGenerateRoundsDialog() override;

    // Getters for dialog values (in milliseconds for durations)
    int desiredDurationMs() const;
    int levelDurationMs() const;
    int expectedBuyins() const;
    int expectedRebuys() const;
    int expectedAddons() const;
    int breakDurationMs() const;
    int anteType() const;
    double anteSbRatio() const;

private Q_SLOTS:
    void on_anteTypeChanged(int index);
};
