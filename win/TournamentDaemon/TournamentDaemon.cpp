// This is the main DLL file.

#include "TournamentDaemon.h"
#include "../../tournamentd/logger.hpp"

using namespace System::Threading;

TBWin::TournamentDaemon::TournamentDaemon() : port(0), running(false), tourney(new tournament), thread(gcnew Thread(gcnew ThreadStart(this, &TBWin::TournamentDaemon::main)))
{
}

TBWin::TournamentDaemon::~TournamentDaemon()
{
	this->Stop();
	delete tourney;
}

void TBWin::TournamentDaemon::main()
{
	while(running)
	{
		auto quit = tourney->run();
		running = running && !quit;
	}
}

int TBWin::TournamentDaemon::Start(int code)
{
	tourney->authorize(code);
	auto service(tourney->listen(nullptr));

	// server is listening. mark as running and run in background
	running = true;
	thread->IsBackground = true;
	thread->Start();

	// store the port
	port = service.second;

	// return the listening port, for subsequent connection
	return port;
}

void TBWin::TournamentDaemon::Publish(System::String^ name)
{
	// TODO: Stop net service

	if (port)
	{
		// TODO: Start net service
	}
}

void TBWin::TournamentDaemon::Stop()
{
	// TODO: Stop net service

	// clear port
	port = 0;

	// signal thread execution to stop
	running = false;

	// join thread
	thread->Join();
}