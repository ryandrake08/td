#include "TournamentObject.h"
#include "../../tournamentd/tournament.hpp"

TBWin::TournamentObject::TournamentObject() : tourney(new tournament)
{
}

TBWin::TournamentObject::~TournamentObject()
{
	delete tourney;
}

int TBWin::TournamentObject::Start(int code)
{
	tourney->authorize(code);
	auto service(tourney->listen(nullptr));
	return service.second;
}

bool TBWin::TournamentObject::Run()
{
	return tourney->run();
}
