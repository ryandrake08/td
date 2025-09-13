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

    int previousBlindLevel { -1 };
    bool previousOnBreak { false };
    int previousTimeRemaining { -1 };
    int previousBreakTimeRemaining { -1 };

    explicit impl(TBSoundPlayer* parent) :
        startSound(new QSoundEffect(parent)),
        nextSound(new QSoundEffect(parent)),
        breakSound(new QSoundEffect(parent)),
        warningSound(new QSoundEffect(parent)),
        rebalanceSound(new QSoundEffect(parent))
    {
        // Load sound files from Resources (use WAV format for Qt cross-platform compatibility)
        this->startSound->setSource(QUrl("qrc:/s_start.wav"));
        this->nextSound->setSource(QUrl("qrc:/s_next.wav"));
        this->breakSound->setSource(QUrl("qrc:/s_break.wav"));
        this->warningSound->setSource(QUrl("qrc:/s_warning.wav"));
        this->rebalanceSound->setSource(QUrl("qrc:/s_rebalance.wav"));
    }
};

TBSoundPlayer::TBSoundPlayer(const TournamentSession& session, QObject* parent) : QObject(parent), pimpl(new impl(this))
{
    // Connect to session state changes
    QObject::connect(&session, &TournamentSession::stateChanged, this, &TBSoundPlayer::on_stateChanged);

    // Connect to player movement updates
    QObject::connect(&session, &TournamentSession::playerMovementsUpdated, this, &TBSoundPlayer::on_playerMovementsUpdated);
}

TBSoundPlayer::~TBSoundPlayer() = default;

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
}

void TBSoundPlayer::on_playerMovementsUpdated(const QVariantList& movements)
{
    Q_UNUSED(movements)

    // Play rebalance sound when player movements occur (equivalent to macOS kMovementsUpdatedNotification)
    pimpl->rebalanceSound->play();
}