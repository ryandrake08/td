using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Web.Script.Serialization;

namespace TBWin
{
    public static class DictionaryExtension
    {
        // This is the extension method.
        // The first parameter takes the "this" modifier
        // and specifies the type for which the method is defined.
        public static void AddRangeAndUpdate<T, S>(this Dictionary<T, S> source, Dictionary<T, S> collection)
        {
            if (collection == null)
            {
                throw new ArgumentNullException("Collection is null");
            }

            foreach (var item in collection)
            {
                if (!source.ContainsKey(item.Key))
                {
                    source.Add(item.Key, item.Value);
                }
                else if(!source[item.Key].Equals(item.Value))
                {
                    source[item.Key] = item.Value;
                }
            }
        }
    }

    class TournamentSession : IDisposable
    {
        // Constructor
        public TournamentSession()
        {
            _serializer = new JavaScriptSerializer();
            _state = new Dictionary<string,dynamic>();
            _client = new TcpClient();
            _commandHandlers = new Dictionary<long, CommandHandler>();
        }

        // Get all tournament configuration and state
        public Dictionary<string,dynamic> State
        {
            get { return _state; }
        }

        // Client identifier (used for authenticating with servers)
        public static int ClientIdentifier()
        {
            if(Properties.Settings.Default.clientIdentifier == 0)
            {
                var rand = new Random();
                var cid = rand.Next(10000, 100000);
                Properties.Settings.Default.clientIdentifier = cid;
            }

            return Properties.Settings.Default.clientIdentifier;
        }

        // Connect to hostname/port
        public async Task Connect(string hostname, int port)
        {
            await _client.ConnectAsync(hostname, port);
            await StartReader();
        }

        // Connect to IPAddress/port
        public async Task Connect(IPAddress address, int port)
        {
            await _client.ConnectAsync(address, port);
            await StartReader();
        }

        public void SelectiveConfigureAndUpdate(Dictionary<string,dynamic> config, ref Dictionary<string,dynamic> newConfig)
        {
            var configToSend = config.Except(_state);
            if(configToSend.Count() > 0)
            {
                System.Diagnostics.Debug.WriteLine("Sending " + configToSend.Count() + " configuration items");
                // TODO: Call Configure()
            }
        }

        // COMMANDS

        public async Task CheckAuthorized(Action<bool> action)
        {
            await SendCommand("check_authorized", delegate (Dictionary<string,dynamic> obj)
            {
                var authorized = obj["authorized"];
                _state["authorized"] = authorized;
                if (action != null)
                {
                    action(authorized);
                }
            });
        }

        public async Task GetState(Action<Dictionary<string,dynamic>> action)
        {
            await SendCommand("get_state", delegate (Dictionary<string, dynamic> obj)
            {
                if (action != null)
                {
                    action(obj);
                }
            });
        }

        public async Task GetConfig(Action<Dictionary<string, dynamic>> action)
        {
            await SendCommand("get_config", delegate (Dictionary<string, dynamic> obj)
            {
                if (action != null)
                {
                    action(obj);
                }
            });
        }

        public async Task Configure(Dictionary<string, dynamic> config, Action<Dictionary<string, dynamic>> action)
        {
            await SendCommand("configure", config, delegate (Dictionary<string, dynamic> obj)
            {
                if (action != null)
                {
                    action(obj);
                }
            });
        }

        public async Task StartGameAt(DateTime datetime)
        {
            var obj = new Dictionary<string, dynamic>
            {
                { "start_at", datetime }
            };
            await SendCommand("start_game", obj);
        }

        public async Task StartGame()
        {
            await SendCommand("start_game");
        }

        public async Task StopGame()
        {
            await SendCommand("stop_game");
        }

        public async Task PauseGame()
        {
            await SendCommand("pause_game");
        }

        public async Task ResumeGame()
        {
            await SendCommand("resume_game");
        }

        public async Task TogglePauseGame()
        {
            await SendCommand("toggle_pause_game");
        }

        public async Task SetPreviousLevel(Action<long> action)
        {
            await SendCommand("set_previous_level", delegate (Dictionary<string, dynamic> obj)
            {
                var level = obj["blind_level_changed"];
                if (action != null)
                {
                    action(level);
                }
            });
        }

        public async Task SetNextLevel(Action<long> action)
        {
            await SendCommand("set_next_level", delegate (Dictionary<string, dynamic> obj)
            {
                var level = obj["blind_level_changed"];
                if (action != null)
                {
                    action(level);
                }
            });
        }

        public async Task SetActionClock(long milliseconds)
        {
            var obj = new Dictionary<string, dynamic>
            {
                { "duration", milliseconds }
            };
            await SendCommand("set_action_clock", obj);
        }

        public async Task ClearActionClock()
        {
            await SendCommand("set_action_clock");
        }

        public async Task GenBlindLevels(int count, long duration, long? breakDuration, double blindIncreaseFactor)
        {
            var obj = new Dictionary<string, dynamic>
            {
                { "count", count },
                { "duration", duration },
                { "break_duration", breakDuration },
                { "blind_increase_factor", blindIncreaseFactor }
            };
            await SendCommand("gen_blind_levels", obj);
        }

        public async Task FundPlayer(string playerId, int sourceId)
        {
            var obj = new Dictionary<string, dynamic>
            {
                { "player_id", playerId },
                { "source_id", sourceId }
            };
            await SendCommand("fund_player", obj);
        }

        public async Task PlanSeating(int expectedPlayers)
        {
            var obj = new Dictionary<string, dynamic>
            {
                { "max_expected_players", expectedPlayers }
            };
            await SendCommand("plan_seating", obj);
        }

        public async Task SeatPlayer(string playerId, Action<string,int,int> action)
        {
            var obj = new Dictionary<string, dynamic>
            {
                { "player_id", playerId }
            };

            await SendCommand("seat_player", delegate (Dictionary<string, dynamic> obj2)
            {
                var playerSeated = obj2["player_seated"];
                if (action != null)
                {
                    action(playerSeated["player_id"], playerSeated["table_number"], playerSeated["seat_number"]);
                }
            });
        }

        public async Task UnseatPlayer(string playerId, Action<string, int, int> action)
        {
            var obj = new Dictionary<string, dynamic>
            {
                { "player_id", playerId }
            };

            await SendCommand("unseat_player", delegate (Dictionary<string, dynamic> obj2)
            {
                var playerUnseated = obj2["player_unseated"];
                if (action != null)
                {
                    action(playerUnseated["player_id"], playerUnseated["table_number"], playerUnseated["seat_number"]);
                }
            });
        }

        public async Task BustPlayer(string playerId, Action<ArrayList> action)
        {
            var obj = new Dictionary<string, dynamic>
            {
                { "player_id", playerId }
            };

            await SendCommand("bust_player", delegate (Dictionary<string, dynamic> obj2)
            {
                var playersMoved = obj["players_moved"];
                if (action != null)
                {
                    action(playersMoved);
                }
            });
        }

        // IDisposable

        public void Dispose()
        {
            _client.Close();
        }

        // Async reader loop
        private async Task StartReader()
        {
            // TODO: Handle connection
            await CheckAuthorized(delegate (bool val)
            {
                if (val)
                {
                    System.Diagnostics.Debug.WriteLine("CheckAuthorized returned true");
                }
                else
                {
                    System.Diagnostics.Debug.WriteLine("CheckAuthorized returned false");
                }
            });

            using (StreamReader reader = new StreamReader(_client.GetStream(), Encoding.UTF8))
            {
                string line;
                while ((line = await reader.ReadLineAsync()) != null)
                {
                    var obj = _serializer.Deserialize<Dictionary<string, dynamic>>(line);

                    // Check for error
                    if (obj.ContainsKey("error"))
                    {
                        // Find and remove the error
                        string error = obj["error"];
                        obj.Remove("error");

                        // TODO: Throw something
                        System.Diagnostics.Debug.WriteLine("tournamentd returned error: " + error);
                    }

                    if (obj.ContainsKey("echo"))
                    {
                        // Find and remove the command key
                        var cmdkey = obj["echo"];
                        obj.Remove("echo");

                        // Look up handler
                        var handler = _commandHandlers[cmdkey];
                        if (handler != null)
                        {
                            // Remove handler from our dictionary
                            _commandHandlers.Remove(cmdkey);
                            handler(obj);
                        }
                    }
                    else
                    {
                        _state.AddRangeAndUpdate(obj);
                    }
                }
            }

            // TODO: Handle disconnection
            System.Diagnostics.Debug.WriteLine("StreamReader finished (client disconnection?)");
        }

        // Command delegates
        delegate void CommandHandler(Dictionary<string, dynamic> obj);
        private Dictionary<long, CommandHandler> _commandHandlers;
        private static long _incrementingKey = 0;

        // Command sender without argument
        private async Task SendCommand(string command, CommandHandler handler=null)
        {
            await SendCommand(command, new Dictionary<string, dynamic>(), handler);
        }

        // Command sender with argument
        private async Task SendCommand(string command, Dictionary<string, dynamic> obj, CommandHandler handler=null)
        {
            // Append to every command: authentication
            obj["authenticate"] = ClientIdentifier();

            // Append to every command: command key
            obj["echo"] = _incrementingKey;

            // Add delegate to our dicationary
            _commandHandlers[_incrementingKey] = handler;

            // Increment key
            _incrementingKey++;

            // Send asynchronously
            using (var writer = new StreamWriter(_client.GetStream(), new UTF8Encoding(false), 1024, true))
            {
                await writer.WriteLineAsync(command + ' ' + _serializer.Serialize(obj));
            }
        }

        // Fields
        private JavaScriptSerializer _serializer;
        private TcpClient _client;
        private Dictionary<string,dynamic> _state;
    }
}
