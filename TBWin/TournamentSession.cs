using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Web.Script.Serialization;

namespace TBWin
{
    public static class DictionaryExtension
    {
        public static bool DeepContains<T,S>(this IDictionary<T,S> source, KeyValuePair<T,S> item)
        {
            if(source.ContainsKey(item.Key))
            {
                var value = source[item.Key];
                if(value.Equals(item.Value))
                {
                    return true;
                }
            }
            return false;
        }

        public static void AddRange<T,S>(this IDictionary<T,S> source, ICollection<KeyValuePair<T,S>> collection)
        {
            if (collection == null)
            {
                throw new ArgumentNullException("Collection is null");
            }

            foreach (var item in collection)
            {
                if (!source.Contains(item))
                {
                    source[item.Key] = item.Value;
                }
            }
        }

        public static IDictionary<T,S> Except<T,S>(this ICollection<KeyValuePair<T,S>> source, ICollection<KeyValuePair<T,S>> collection)
        {
            if (collection == null)
            {
                throw new ArgumentNullException("Collection is null");
            }

            var ret = new Dictionary<T, S>();

            foreach (var item in source)
            {
                if (!collection.Contains(item))
                {
                    ret[item.Key] = item.Value;
                }
            }

            return ret;
        }
    }

    class TournamentSession : IDisposable
    {
        // Constructor
        public TournamentSession()
        {
            _state = new Dictionary<string,dynamic>();
            _client = new TcpClient();
            _commandHandlers = new Dictionary<long, CommandHandler>();
        }

        // Get all tournament configuration and state
        public IDictionary<string,dynamic> State
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

        public async Task SelectiveConfigure(
            ICollection<KeyValuePair<string,dynamic>> config,
            ICollection<KeyValuePair<string, dynamic>> referenceConfig,
            Action<IDictionary<string, dynamic>> action)
        {
            var configToSend = config.Except(_state);
            if(configToSend.Count > 0)
            {
                System.Diagnostics.Debug.WriteLine("Sending " + configToSend.Count + " configuration items");
                await Configure(configToSend, delegate (IDictionary<string, dynamic> returnedConfig)
                {
                    var differences = returnedConfig.Except(referenceConfig);
                    if (differences.Count > 0)
                    {
                        System.Diagnostics.Debug.WriteLine("Updating existing config with " + differences.Count + " configuration items");
                        if (action != null)
                        {
                            action(differences);
                        }
                    }
                });
            }
        }

        // COMMANDS

        public async Task CheckAuthorized(Action<bool> action)
        {
            await SendCommand("check_authorized", delegate (IDictionary<string,dynamic> obj)
            {
                var authorized = obj["authorized"];
                _state["authorized"] = authorized;
                if (action != null)
                {
                    action(authorized);
                }
            });
        }

        public async Task GetState(Action<IDictionary<string,dynamic>> action)
        {
            await SendCommand("get_state", delegate (IDictionary<string, dynamic> obj)
            {
                if (action != null)
                {
                    action(obj);
                }
            });
        }

        public async Task GetConfig(Action<IDictionary<string, dynamic>> action)
        {
            await SendCommand("get_config", delegate (IDictionary<string, dynamic> obj)
            {
                if (action != null)
                {
                    action(obj);
                }
            });
        }

        public async Task Configure(ICollection<KeyValuePair<string, dynamic>> config, Action<IDictionary<string, dynamic>> action)
        {
            await SendCommand("configure", config, delegate (IDictionary<string, dynamic> obj)
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
            await SendCommand("set_previous_level", delegate (IDictionary<string, dynamic> obj)
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
            await SendCommand("set_next_level", delegate (IDictionary<string, dynamic> obj)
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

            await SendCommand("seat_player", delegate (IDictionary<string, dynamic> obj2)
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

            await SendCommand("unseat_player", delegate (IDictionary<string, dynamic> obj2)
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

            await SendCommand("bust_player", delegate (IDictionary<string, dynamic> obj2)
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
                var serializer = new JavaScriptSerializer();

                string line;
                while ((line = await reader.ReadLineAsync()) != null)
                {
                    var obj = serializer.Deserialize<IDictionary<string, dynamic>>(line);

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
                        _state.AddRange(obj);
                    }
                }
            }

            // TODO: Handle disconnection
            System.Diagnostics.Debug.WriteLine("StreamReader finished (client disconnection?)");
        }

        // Command delegates
        delegate void CommandHandler(IDictionary<string, dynamic> obj);
        private IDictionary<long, CommandHandler> _commandHandlers;
        private static long _incrementingKey = 0;

        // Command sender without argument
        private async Task SendCommand(string command, CommandHandler handler=null)
        {
            await SendCommand(command, new Dictionary<string, dynamic>(), handler);
        }

        // Command sender with argument
        private async Task SendCommand(string command, ICollection<KeyValuePair<string, dynamic>> obj, CommandHandler handler=null)
        {
            // Append to every command: authentication
            obj.Add(new KeyValuePair<string, dynamic>("authenticate", ClientIdentifier()));

            // Append to every command: command key
            obj.Add(new KeyValuePair<string, dynamic>("echo", _incrementingKey));

            // Add delegate to our dicationary
            _commandHandlers[_incrementingKey] = handler;

            // Increment key
            _incrementingKey++;

            // Crease serializer
            var serializer = new JavaScriptSerializer();

            // Send asynchronously
            using (var writer = new StreamWriter(_client.GetStream(), new UTF8Encoding(false), 1024, true))
            {
                await writer.WriteLineAsync(command + ' ' + serializer.Serialize(obj));
            }
        }

        // Fields
        private TcpClient _client;
        private IDictionary<string,dynamic> _state;
    }
}
