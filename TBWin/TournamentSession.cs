using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace TBWin
{
    class TournamentSession : IDisposable
    {
        // Constructor
        public TournamentSession()
        {
            _state = new Newtonsoft.Json.Linq.JObject();
            _client = new TcpClient();
            _commandHandlers = new Dictionary<int, CommandHandler>();
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
            await CheckAuthorized(delegate(bool val) {
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
                    dynamic json = JsonConvert.DeserializeObject(line);

                    // Look for command key
                    int? cmdkey = json["echo"];
                    if(cmdkey == null)
                    {
                        // TODO: [json dictionaryWithChangesFromDictionary:[self state]]

                        // TODO: Update state
                    }
                    else
                    {
                        // Remove command key response
                        json.Remove("echo");

                        // Look up handler
                        CommandHandler handler = _commandHandlers[cmdkey.Value];
                        if(handler != null)
                        {
                            // Find and remove the error
                            string error = json["error"];
                            json.Remove("error");

                            // Call hander
                            handler(json, error);

                            // Remove handler from our dictionary
                        }
                    }
                }
            }

            // TODO: Handle disconnection
        }

        // Command delegates
        delegate void CommandHandler(dynamic obj, string error);
        private Dictionary<int, CommandHandler> _commandHandlers;
        private static int _incrementingKey = 0;

        // Command sender
        private async Task SendCommand(string command)
        {
            // Send asynchronously
            using (StreamWriter writer = new StreamWriter(_client.GetStream(), new UTF8Encoding(false), 1024, true))
            {
                await writer.WriteLineAsync(command);
            }
        }

        // Command sender with argument
        private async Task SendCommand(string command, dynamic obj, CommandHandler handler)
        {
            // Add extra stuff to each command
            dynamic json = obj;
            if(json == null)
            {
                json = new Dictionary<string,int>();
            }

            // Append to every command: authentication
            int cid = ClientIdentifier();
            json["authenticate"] = cid;

            // Append to every command: command key
            int key = _incrementingKey++;
            json["echo"] = key;

            // Add delegate to our dicationary
            _commandHandlers[key] = handler;

            // Serialize
            string arg = JsonConvert.SerializeObject(json);

            // Send
            await SendCommand(command + ' ' + arg);
        }

        // Return the random client identifier for this device
        public static int ClientIdentifier()
        {
            if(Properties.Settings.Default.clientIdentifier == 0)
            {
                Random rand = new Random();
                int cid = rand.Next(10000, 100000);
                Properties.Settings.Default.clientIdentifier = cid;
            }

            return Properties.Settings.Default.clientIdentifier;
        }

        // COMMANDS

        public async Task CheckAuthorized(Action<bool> action)
        {
            await SendCommand("check_authorized", null, delegate (dynamic obj, string error)
            {
                if(error != null)
                {
                    System.Diagnostics.Debug.WriteLine("CheckAuthorized: " + error);
                }
                else
                {
                    _state["authorized"] = obj["authorized"];
                    if(action != null)
                    {
                        action(obj["authorized"].Value);
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
        private dynamic _state;
    }
}
