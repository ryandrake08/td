#include "TBSoundPlayer.hpp"

#include "TournamentSession.hpp"

#include <QDebug>
#include <QFile>
#include <QSoundEffect>
#include <QUrl>

struct TBSoundPlayer::impl
{
    QSoundEffect* startSound;
    QSoundEffect* nextSound;
    QSoundEffect* breakSound;
    QSoundEffect* warningSound;
    QSoundEffect* rebalanceSound;

    int previousBlindLevel;
    bool previousOnBreak;
    int previousTimeRemaining;
    int previousBreakTimeRemaining;
};

TBSoundPlayer::TBSoundPlayer(QObject* parent) : QObject(parent), pimpl(new impl())
{
    // Initialize sound effects with Qt parent-child ownership
    pimpl->startSound = new QSoundEffect(this);
    pimpl->nextSound = new QSoundEffect(this);
    pimpl->breakSound = new QSoundEffect(this);
    pimpl->warningSound = new QSoundEffect(this);
    pimpl->rebalanceSound = new QSoundEffect(this);

    // Load sound files from Resources (use WAV format for Qt cross-platform compatibility)
    pimpl->startSound->setSource(QUrl("qrc:/Resources/s_start.wav"));
    pimpl->nextSound->setSource(QUrl("qrc:/Resources/s_next.wav"));
    pimpl->breakSound->setSource(QUrl("qrc:/Resources/s_break.wav"));
    pimpl->warningSound->setSource(QUrl("qrc:/Resources/s_warning.wav"));
    pimpl->rebalanceSound->setSource(QUrl("qrc:/Resources/s_rebalance.wav"));

    // Sound files are now loading correctly with qrc:/ URLs

    // Initialize state tracking
    pimpl->previousBlindLevel = -1;
    pimpl->previousOnBreak = false;
    pimpl->previousTimeRemaining = -1;
    pimpl->previousBreakTimeRemaining = -1;
}

TBSoundPlayer::~TBSoundPlayer() = default;

void TBSoundPlayer::setSession(const TournamentSession& session)
{
    // Connect to session state changes
    QObject::connect(&session, &TournamentSession::stateChanged, this, &TBSoundPlayer::on_stateChanged);

    // Connect to player movement updates
    QObject::connect(&session, &TournamentSession::playerMovementsUpdated, this, &TBSoundPlayer::on_playerMovementsUpdated);
}

void TBSoundPlayer::on_stateChanged(const QString& key, const QVariant& value)
{
    // Handle blind level changes (tournament start/next round)
    if(key == "current_blind_level")
    {
        int newBlindLevel = value.toInt();

        if(pimpl->previousBlindLevel != -1)
        { // Not the first update
            if(pimpl->previousBlindLevel == 0 && newBlindLevel != 0)
            {
                // Round zero to non-zero: tournament start
                pimpl->startSound->play();
            }
            else if(pimpl->previousBlindLevel != 0 && newBlindLevel != 0 && pimpl->previousBlindLevel != newBlindLevel)
            {
                // Round non-zero to different non-zero: next/previous round
                pimpl->nextSound->play();
            }
            // Note: non-zero to zero (restart) has no sound per macOS implementation
        }

        pimpl->previousBlindLevel = newBlindLevel;
    }

    // Handle break changes
    else if(key == "on_break")
    {
        bool newOnBreak = value.toBool();

        if(!pimpl->previousOnBreak && newOnBreak)
        {
            // Break started
            pimpl->breakSound->play();
        }

        pimpl->previousOnBreak = newOnBreak;
    }

    // Handle time remaining warnings
    else if(key == "time_remaining")
    {
        int newTimeRemaining = value.toInt();

        if(pimpl->previousTimeRemaining > TournamentSession::kAudioWarningTime && newTimeRemaining <= TournamentSession::kAudioWarningTime && newTimeRemaining != 0)
        {
            pimpl->warningSound->play();
        }

        pimpl->previousTimeRemaining = newTimeRemaining;
    }

    // Handle break time remaining warnings
    else if(key == "break_time_remaining")
    {
        int newBreakTimeRemaining = value.toInt();

        if(pimpl->previousBreakTimeRemaining > TournamentSession::kAudioWarningTime && newBreakTimeRemaining <= TournamentSession::kAudioWarningTime && newBreakTimeRemaining != 0)
        {
            pimpl->warningSound->play();
        }

        pimpl->previousBreakTimeRemaining = newBreakTimeRemaining;
    }

    // Handle player movements/rebalancing
    // Note: The macOS version listens for kMovementsUpdatedNotification
    // We could add this when movements are implemented, or listen for relevant state changes
}

void TBSoundPlayer::on_playerMovementsUpdated(const QVariantList& movements)
{
    Q_UNUSED(movements)

    // Play rebalance sound when player movements occur (equivalent to macOS kMovementsUpdatedNotification)
    pimpl->rebalanceSound->play();
}