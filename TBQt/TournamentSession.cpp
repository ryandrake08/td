#include "TournamentSession.hpp"

#include <QSettings>
#include <QString>

#include <random>

int TournamentSession::client_identifier()
{
    const auto key(QStringLiteral("clientIdentifier"));

    // if we don't already have one
    QSettings settings;
    if(!settings.contains(key))
    {
        // generate a new identifier
        std::random_device rd;
        std::default_random_engine gen(rd());
        std::uniform_int_distribution<int> dist(10000, 99999);
        auto cid(dist(gen));

        // store it (client id persists in settings)
        settings.setValue(key, cid);
    }

    return settings.value(key).toInt();
}
