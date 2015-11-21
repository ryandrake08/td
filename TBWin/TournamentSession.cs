using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;
using Newtonsoft.Json;

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
            _state = new Dictionary<string,dynamic>();
            _client = new TcpClient();
            _commandHandlers = new Dictionary<long, CommandHandler>();
        }

        // Connect to hostname/port
        public async void Connect(string hostname, int port)
        {
            await _client.ConnectAsync(hostname, port);
            await StartReader();
        }

        // Connect to IPAddress/port
        public async void Connect(IPAddress address, int port)
        {
            await _client.ConnectAsync(address, port);
            await StartReader();
        }

        // Async reader loop
        private async Task StartReader()
        {
            // TODO: Handle connection
            await CheckAuthorized(delegate(bool val)
            {
                if(val)
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
                    var json = JsonConvert.DeserializeObject<Dictionary<string,dynamic>>(line);
                    try
                    {
                        // Look for command key
                        var cmdkey = json["echo"];

                        // Remove command key response
                        json.Remove("echo");

                        // Look up handler
                        var handler = _commandHandlers[cmdkey];
                        if(handler != null)
                        {
                            try
                            {
                                // Find and remove the error
                                var error = json["error"];
                                json.Remove("error");

                                // Call hander
                                handler(json, error);
                            }
                            catch (KeyNotFoundException)
                            {
                                handler(json, null);
                            }

                            // Remove handler from our dictionary
                            _commandHandlers.Remove(cmdkey);
                        }
                    }
                    catch (KeyNotFoundException)
                    {
                        _state.AddRangeAndUpdate(json);
                    }
                }
            }

            // TODO: Handle disconnection
        }

        // Command delegates
        delegate void CommandHandler(Dictionary<string,dynamic> obj, string error);
        private Dictionary<long, CommandHandler> _commandHandlers;
        private static long _incrementingKey = 0;

        // Command sender
        private async Task SendCommand(string command)
        {
            // Send asynchronously
            using (var writer = new StreamWriter(_client.GetStream(), new UTF8Encoding(false), 1024, true))
            {
                await writer.WriteLineAsync(command);
            }
        }

        // Command sender with argument
        private async Task SendCommand(string command, Dictionary<string,dynamic> obj, CommandHandler handler)
        {
            // Add extra stuff to each command
            var json = obj;
            if(json == null)
            {
                json = new Dictionary<string,dynamic>();
            }

            // Append to every command: authentication
            var cid = ClientIdentifier();
            json["authenticate"] = cid;

            // Append to every command: command key
            var key = _incrementingKey++;
            json["echo"] = key;

            // Add delegate to our dicationary
            _commandHandlers[key] = handler;

            // Serialize
            var arg = JsonConvert.SerializeObject(json);

            // Send
            await SendCommand(command + ' ' + arg);
        }

        // Return the random client identifier for this device
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

        // COMMANDS

        public async Task CheckAuthorized(Action<bool> action)
        {
            await SendCommand("check_authorized", null, delegate (Dictionary<string,dynamic> obj, string error)
            {
                if(error != null)
                {
                    System.Diagnostics.Debug.WriteLine("CheckAuthorized: " + error);
                }
                else
                {
                    try
                    {
                        var authorized = obj["authorized"];
                        _state["authorized"] = authorized;
                        action(authorized);
                    }
                    catch (KeyNotFoundException)
                    {
                        System.Diagnostics.Debug.WriteLine("CheckAuthorized: authorized key not found");
                    }
                }
            });
        }

        public void Dispose()
        {
            ((IDisposable)_client).Dispose();
        }

        // Fields
        private TcpClient _client;
        private Dictionary<string,dynamic> _state;
    }
}
