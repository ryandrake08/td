#pragma once

#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QObject>

class TBCurrency : public QObject
{
    Q_OBJECT

    // pimpl
    struct impl;

public:
    // Public static interface - only expose what's actually used
    static QStringList supportedCodes();
    static QStringList supportedNames();
    static QString defaultCurrencyCode();
    static QString formatAmount(double amount, const QString& currencyCode);
    static QString currencyName(const QString& currencyCode);
    static QString currencyImagePath(const QString& currencyCode);
    static QPixmap currencyPixmap(const QString& currencyCode);
    static bool isValidCurrency(const QString& currencyCode);
};