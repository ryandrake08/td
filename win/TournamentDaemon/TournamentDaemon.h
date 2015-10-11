// TournamentDaemon.h

#pragma once

namespace TBWin {

	public ref class TournamentDaemon
	{
	public:
		// Start daemon with auth code, returns listening port
		int Start(int code);

		// Publish over Bonjour
		void Publish(System::String^ name);

		// Stop daemon
		void Stop();
	};
}
