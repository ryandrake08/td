#pragma once

class tournament;

namespace TBWin
{
	public ref class TournamentObject
	{
		// Tournament object
		tournament* tourney;

	public:
		TournamentObject();
		~TournamentObject();

		// Start daemon with auth code, returns listening port
		int Start(int code);

		// Run one iteration, returns true if we need to quit
		bool Run();
	};
}
