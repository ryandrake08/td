#include "TBResultsModel.hpp"

#include "TBCurrency.hpp"
#include "TournamentSession.hpp"

TBResultsModel::TBResultsModel(const TournamentSession& session, QObject* parent) : TBVariantListTableModel(parent), m_session(session)
{
    // set headers for this kind of model
    this->addHeader("place", QObject::tr("Place"));
    this->addHeader("name", QObject::tr("Player Name"));
    this->addHeader("payout", QObject::tr("Payout"));

    // observe session state
    QObject::connect(&session, SIGNAL(stateChanged(const QString&, const QVariant&)), this, SLOT(on_stateChanged(const QString&, const QVariant&)));
}

void TBResultsModel::on_stateChanged(const QString& key, const QVariant& value)
{
    if(key == "results")
    {
        this->setListData(value.toList());
    }
}

QVariant TBResultsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // Handle display role for place column to show ordinals (1st, 2nd, 3rd, etc.)
    if (role == Qt::DisplayRole && index.column() == 0) // "place" column
    {
        QVariantMap rowData = this->getRowData(index.row());
        int place = rowData["place"].toInt();

        if (place > 0)
        {
            // Convert to ordinal format (1st, 2nd, 3rd, etc.)
            QString suffix;
            if (place % 100 >= 11 && place % 100 <= 13)
            {
                // Special case for 11th, 12th, 13th
                suffix = "th";
            }
            else
            {
                switch (place % 10)
                {
                    case 1: suffix = "st"; break;
                    case 2: suffix = "nd"; break;
                    case 3: suffix = "rd"; break;
                    default: suffix = "th"; break;
                }
            }

            return QString("%1%2").arg(place).arg(suffix);
        }

        return rowData["place"]; // Fallback to original value
    }
    // Handle display role for payout column to extract amount from monetary structure
    else if (role == Qt::DisplayRole && index.column() == 2) // "payout" column
    {
        QVariantMap rowData = this->getRowData(index.row());
        QVariantMap payoutData = rowData["payout"].toMap();

        if (!payoutData.isEmpty())
        {
            // Extract amount from monetary_value_nocurrency structure
            double amount = payoutData["amount"].toDouble();

            // Format using tournament's payout currency
            QString payoutCurrency = m_session.state().value("payout_currency", TBCurrency::defaultCurrencyCode()).toString();

            return TBCurrency::formatAmount(amount, payoutCurrency);
        }

        // Fallback if no payout data - use tournament currency for consistency
        QString payoutCurrency = m_session.state().value("payout_currency", TBCurrency::defaultCurrencyCode()).toString();
        return TBCurrency::formatAmount(0.0, payoutCurrency);
    }

    // Handle text alignment for all columns
    if (role == Qt::TextAlignmentRole)
    {
        if (index.column() == 1) // "Player Name" column
        {
            // Left-justify and center vertically for Player Name
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
        else if (index.column() == 2) // "Payout" column
        {
            // Right-justify and center vertically for Payout (currency amounts)
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        }
        else // Place column
        {
            // Center horizontally and vertically for Place
            return static_cast<int>(Qt::AlignCenter);
        }
    }

    // For all other cases, use the base implementation
    return TBVariantListTableModel::data(index, role);
}