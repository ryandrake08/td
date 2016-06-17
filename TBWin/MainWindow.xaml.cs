using System;
using System.Collections.Generic;
using System.Net;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using Newtonsoft.Json.Schema;

namespace TBWin
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public sealed partial class MainWindow : IDisposable
    {
        public MainWindow()
        {
            // Register global command bindings
            CommandManager.RegisterClassCommandBinding(typeof(Window), new CommandBinding(Commands.Exit, ExitCommand_Executed));
            CommandManager.RegisterClassCommandBinding(typeof(Window), new CommandBinding(Commands.Authorize, AuthorizeCommand_Executed));
            CommandManager.RegisterClassCommandBinding(typeof(Window), new CommandBinding(Commands.Setup, SetupCommand_Executed));
            CommandManager.RegisterClassCommandBinding(typeof(Window), new CommandBinding(Commands.Plan, PlanCommand_Executed));
            CommandManager.RegisterClassCommandBinding(typeof(Window), new CommandBinding(Commands.Results, ResultsCommand_Executed));
            CommandManager.RegisterClassCommandBinding(typeof(Window), new CommandBinding(Commands.Rebalance, RebalanceCommand_Executed));
            CommandManager.RegisterClassCommandBinding(typeof(Window), new CommandBinding(Commands.Display, DisplayCommand_Executed));
            CommandManager.RegisterClassCommandBinding(typeof(Window), new CommandBinding(Commands.PlayPause, PlayPauseCommand_Executed));
            CommandManager.RegisterClassCommandBinding(typeof(Window), new CommandBinding(Commands.PreviousRound, PreviousRoundCommand_Executed));
            CommandManager.RegisterClassCommandBinding(typeof(Window), new CommandBinding(Commands.NextRound, NextRoundCommand_Executed));
            CommandManager.RegisterClassCommandBinding(typeof(Window), new CommandBinding(Commands.CallClock, CallClockCommand_Executed));

            _daemon = new TournamentDaemon();
            int port = _daemon.Start(TournamentSession.ClientIdentifier());
            _daemon.Publish("TBWin");
            _session = new TournamentSession();
            _connectTask = _session.Connect(IPAddress.Loopback, port);
            _document = new JsonDocument();

            InitializeComponent();
        }

        public TournamentSession Session
        {
            get { return _session;  }
        }

        private async void ConfigureSession()
        {
            if (_document.Content != null)
            {
                await _session.Configure(_document.Content, delegate (IDictionary<string, dynamic> newConfig)
                {
                    _document.Content.AddRange(newConfig);
                });
            }
        }

        public void LoadSession(string path)
        {
            _document = new JsonDocument(path);
            ConfigureSession();
        }

        private void NewCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            _document = new JsonDocument();
            ConfigureSession();
        }

        private void OpenCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            _document.Open();
            ConfigureSession();
        }

        private void SaveCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            _document.Save();
        }

        private void SaveAsCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            _document.SaveNew();
        }

        private void ExitCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void AuthorizeCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var authCodeWindow = new TBAuthCodeWindow();
            authCodeWindow.ShowDialog();
            if (authCodeWindow.DialogResult.HasValue && authCodeWindow.DialogResult.Value)
            {
                MessageBox.Show("User clicked OK");
            }
            else
            {
                MessageBox.Show("User clicked Cancel");
            }
        }

        private void SetupCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var authCodeWindow = new TBConfigurationWindow(_document);
            authCodeWindow.ShowDialog();
            if (authCodeWindow.DialogResult.HasValue && authCodeWindow.DialogResult.Value)
            {
                MessageBox.Show("User clicked OK");
            }
            else
            {
                MessageBox.Show("User clicked Cancel");
            }
        }

        private void PlanCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var authCodeWindow = new TBPlanWindow();
            authCodeWindow.ShowDialog();
            if (authCodeWindow.DialogResult.HasValue && authCodeWindow.DialogResult.Value)
            {
                MessageBox.Show("User clicked OK");
            }
            else
            {
                MessageBox.Show("User clicked Cancel");
            }
        }

        private void ResultsCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
        }

        private void RebalanceCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var movementWindow = new TBMovementWindow();
            movementWindow.DataContext = _session.State;
            movementWindow.Show();
        }

        private void DisplayCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var playerWindow = new TBPlayerWindow(_session);
            playerWindow.Show();
        }

        private async void PlayPauseCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var currentBlindLevel = Session.State["current_blind_level"];
            if (currentBlindLevel != 0)
            {
                await Session.TogglePauseGame();
            }
            else
            {
                await Session.StartGame();
            }
        }

        private async void PreviousRoundCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var currentBlindLevel = Session.State["current_blind_level"];
            if (currentBlindLevel != 0)
            {
                await Session.SetPreviousLevel(null);
            }
        }

        private async void NextRoundCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var currentBlindLevel = Session.State["current_blind_level"];
            if (currentBlindLevel != 0)
            {
                await Session.SetNextLevel(null);
            }
        }

        private async void CallClockCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var currentBlindLevel = Session.State["current_blind_level"];
            if (currentBlindLevel != 0)
            {
                var remaining = Session.State["action_clock_time_remaining"];
                if (remaining == 0)
                {
                    await Session.SetActionClock(TournamentSession.ActionClockRequestTime);
                }
                else
                {
                    await Session.ClearActionClock();
                }
            }
        }

        public void Dispose()
        {
            _connectTask.Wait();
            _daemon.Dispose();
            _session.Dispose();
        }

        private JsonDocument _document;
        private readonly TournamentDaemon _daemon;
        private readonly TournamentSession _session;
        private readonly Task _connectTask;
    }

    public static class Commands
    {
        public static readonly RoutedUICommand Exit = new RoutedUICommand("Exit", "Exit", typeof(Commands));
        public static readonly RoutedUICommand Authorize = new RoutedUICommand("Authorize", "Authorize", typeof(Commands));
        public static readonly RoutedUICommand Setup = new RoutedUICommand("Setup", "Setup", typeof(Commands));
        public static readonly RoutedUICommand Plan = new RoutedUICommand("Plan", "Plan", typeof(Commands));
        public static readonly RoutedUICommand Results = new RoutedUICommand("Results", "Results", typeof(Commands));
        public static readonly RoutedUICommand Rebalance = new RoutedUICommand("Rebalance", "Rebalance", typeof(Commands));
        public static readonly RoutedUICommand Display = new RoutedUICommand("Display", "Display", typeof(Commands));
        public static readonly RoutedUICommand PlayPause = new RoutedUICommand("PlayPause", "PlayPause", typeof(Commands));
        public static readonly RoutedUICommand NextRound = new RoutedUICommand("NextRound", "NextRound", typeof(Commands));
        public static readonly RoutedUICommand PreviousRound = new RoutedUICommand("PreviousRound", "PreviousRound", typeof(Commands));
        public static readonly RoutedUICommand CallClock = new RoutedUICommand("CallClock", "CallClock", typeof(Commands));
    }
}
