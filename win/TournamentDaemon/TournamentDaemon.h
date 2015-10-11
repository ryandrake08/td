// TournamentDaemon.h

#include "../../tournamentd/tournament.hpp"

#pragma once

namespace TBWin {

	public ref class TournamentDaemon
	{
		// Port in use
		int port;

		// Flag to control whether background thread is running
		bool running;

		// Tournament object
		tournament* tourney;

		// Thread
		System::Threading::Thread^ thread;

		// Thread entry point
		void main();

	public:
		TournamentDaemon();
		~TournamentDaemon();

		// Start daemon with auth code, returns listening port
		int Start(int code);

		// Publish over Bonjour
		void Publish(System::String^ name);

		// Stop daemon
		void Stop();
	};
}
