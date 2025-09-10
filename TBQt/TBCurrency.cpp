#include "TBCurrency.hpp"

#include <QApplication>
#include <QLocale>
#include <QMap>
#include <QRegularExpression>

// Private implementation details
struct TBCurrency::impl
{
    enum class Type
    {
        Standard, // Uses Qt locale formatting
        Custom    // Uses custom formatting rules
    };

    struct CurrencyInfo // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        QString code;          // "USD", "EUR", etc.
        QString displayName;   // Localized name (filled at runtime)
        QString imageResource; // Resource path for image
        Type type;             // Standard or Custom
        QString customSymbol;  // For custom currencies only
        QString customFormat;  // For custom currencies only
    };

    static const QMap<QString, CurrencyInfo>& supportedCurrencies()
    {
        static QMap<QString, CurrencyInfo> currencies = initializeCurrencies();
        return currencies;
    }

    static QMap<QString, CurrencyInfo> initializeCurrencies()
    {
        QMap<QString, CurrencyInfo> currencies;

        // Standard currencies - displayName filled at runtime
        currencies["USD"] = { "USD", "", "b_note_dollar", Type::Standard, "", "" };
        currencies["EUR"] = { "EUR", "", "b_note_euro", Type::Standard, "", "" };
        currencies["INR"] = { "INR", "", "b_note_rupee", Type::Standard, "", "" };
        currencies["GBP"] = { "GBP", "", "b_note_sterling", Type::Standard, "", "" };
        currencies["JPY"] = { "JPY", "", "b_note_yen_yuan", Type::Standard, "", "" };
        currencies["CNY"] = { "CNY", "", "b_note_yen_yuan", Type::Standard, "", "" };

        // Custom currencies with localized names
        currencies["XBP"] = { "XBP", QObject::tr("Bucks"), "b_note_dollar", Type::Custom, QObject::tr("Bucks"), "#,##0 ¤" };
        currencies["XPT"] = { "XPT", QObject::tr("Points"), "b_note_dollar", Type::Custom, QObject::tr("Points"), "#,##0 ¤" };

        return currencies;
    }

    static QString formatStandardCurrency(double amount, const QString& currencyCode)
    {
        QLocale locale;

        // Dynamic fraction digits: 0 for whole numbers, 2 for decimals
        bool isWholeNumber = (amount == static_cast<long long>(amount));

        if(isWholeNumber)
        {
            // Format as integer currency
            return locale.toCurrencyString(static_cast<long long>(amount), currencyCode);
        }
        else
        {
            // Format with 2 decimal places
            return locale.toCurrencyString(amount, currencyCode);
        }
    }

    static QString formatCustomCurrency(double amount, const CurrencyInfo& info)
    {
        // Custom currencies use integer formatting with custom symbol
        // Format: "#,##0 ¤" where ¤ is replaced with custom symbol

        QLocale locale;
        QString numberStr = locale.toString(static_cast<long long>(amount));

        // Replace currency placeholder with custom symbol
        QString result = QString("%1 %2").arg(numberStr, info.customSymbol);

        return result;
    }
};

QStringList TBCurrency::supportedCodes()
{
    return impl::supportedCurrencies().keys();
}

QStringList TBCurrency::supportedNames()
{
    QStringList names;
    for(const auto& code : supportedCodes())
    {
        names.append(currencyName(code));
    }
    return names;
}

QString TBCurrency::defaultCurrencyCode()
{
    // Pick a default currency based on user's configured locale
    QLocale locale;
    QString localeCode = locale.currencySymbol(QLocale::CurrencyIsoCode);

    if(isValidCurrency(localeCode))
    {
        return localeCode;
    }
    else
    {
        return "USD"; // Fallback
    }
}

QString TBCurrency::currencyName(const QString& currencyCode)
{
    const auto& currencies = impl::supportedCurrencies();
    auto it = currencies.find(currencyCode);

    if(it == currencies.end())
    {
        return QObject::tr("Unknown");
    }

    const impl::CurrencyInfo& info = it.value();

    if(info.type == impl::Type::Custom)
    {
        return info.displayName; // Already localized
    }
    else
    {
        // Use Qt's locale system for standard currencies
        QLocale locale;
        QString localizedName = locale.currencySymbol(QLocale::CurrencyDisplayName);

        if(localizedName.isEmpty())
        {
            // Fallback to currency code if no localized name available
            return currencyCode;
        }

        return localizedName;
    }
}

QString TBCurrency::formatAmount(double amount, const QString& currencyCode)
{
    const auto& currencies = impl::supportedCurrencies();
    auto it = currencies.find(currencyCode);

    if(it == currencies.end())
    {
        // Fallback to USD formatting for unknown currencies
        return impl::formatStandardCurrency(amount, "USD");
    }

    const impl::CurrencyInfo& info = it.value();

    if(info.type == impl::Type::Standard)
    {
        return impl::formatStandardCurrency(amount, currencyCode);
    }
    else
    {
        return impl::formatCustomCurrency(amount, info);
    }
}

QString TBCurrency::currencyImagePath(const QString& currencyCode)
{
    const auto& currencies = impl::supportedCurrencies();
    auto it = currencies.find(currencyCode);

    if(it != currencies.end())
    {
        return it.value().imageResource;
    }

    // Fallback to dollar image
    return ":/Resources/b_note_dollar.svg";
}

QPixmap TBCurrency::currencyPixmap(const QString& currencyCode)
{
    QString imagePath = currencyImagePath(currencyCode);
    return { imagePath };
}

bool TBCurrency::isValidCurrency(const QString& currencyCode)
{
    return impl::supportedCurrencies().contains(currencyCode);
}