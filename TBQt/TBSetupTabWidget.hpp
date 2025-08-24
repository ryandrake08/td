#pragma once

#include <QWidget>
#include <QVariantMap>

class TBSetupTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TBSetupTabWidget(QWidget* parent = nullptr);
    virtual ~TBSetupTabWidget();

    virtual void setConfiguration(const QVariantMap& configuration) = 0;
    virtual QVariantMap configuration() const = 0;
    virtual bool validateConfiguration() const = 0;

Q_SIGNALS:
    void configurationChanged();
};