using System.Threading;
using ZeroconfService;

namespace TBWin
{
    class TournamentDaemon
    {
        public int Start(int code)
        {
            // tournament object
            TournamentObject tourney = new TournamentObject();

            // start the service and get the port
            _port = tourney.Start(code);

            // server is listening. mark as running
            _running = true;

            // create a background thread
            _thread = new Thread((ThreadStart)delegate
            {
                while(_running)
                {
                    bool quit = tourney.Run();
                    _running = _running && !quit;
                }
            });
            _thread.IsBackground = true;
            _thread.Start();

            // return the listening port, for subsequent connection
            return _port;
        }

        public bool Publish(string name)
        {
            // stop net service
            if(_netService != null)
            {
                _netService.Stop();
                _netService = null;
            }

            // start new net service and publish
            if(_port != 0)
            {
                _netService = new NetService("local.", "_tournbuddy._tcp.", name, _port);
                try
                {
                    _netService.Publish();
                    return true;
                }
                catch(System.DllNotFoundException)
                {
                    // bonjour not installed. just return false to indicate we did not publish
                }
                _netService = null;
            }

            return false;
        }

        public void Stop()
        {
            // stop net service
            if (_netService != null)
            {
                _netService.Stop();
            }

            // clear port
            _port = 0;

            // signal thread execution to stop
            _running = false;

            // join thread
            _thread.Join();
        }

        // Port in use
        private int _port;

        // Flag to control whether background thread is running
        private bool _running;

        // Thread
        private Thread _thread;

        // Net service
        private NetService _netService;
    }
}
