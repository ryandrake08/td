using System;
using System.Collections.Generic;
using System.Net;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace TBWin
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public sealed partial class MainWindow : IDisposable
    {
        public MainWindow()
        {
            _daemon = new TournamentDaemon();
            int port = _daemon.Start(TournamentSession.ClientIdentifier());
            _daemon.Publish("TBWin");
            _session = new TournamentSession();
            _connectTask = _session.Connect(IPAddress.Loopback, port);

            InitializeComponent();
        }

        public JsonDocument Document
        {
            get { return _document; }
            set { _document = value; }
        }

        public TournamentSession Session
        {
            get { return _session;  }
        }

        private void NewCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            _document = new JsonDocument();
        }

        private async void OpenCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            _document.Open();

            await _session.SelectiveConfigure(_document.Content, _document.Content, delegate (IDictionary<string, dynamic> newConfig)
            {
                _document.Content.AddRange(newConfig);
            });
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
            var authCodeWindow = new TBConfigurationWindow();
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

        private void PlayPauseCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
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
        public static readonly RoutedUICommand Exit = new RoutedUICommand("Exit", "Exit", typeof(Commands), new InputGestureCollection() { new KeyGesture(Key.F4, ModifierKeys.Alt) });
        public static readonly RoutedUICommand Authorize = new RoutedUICommand("Authorize", "Authorize", typeof(Commands));
        public static readonly RoutedUICommand Setup = new RoutedUICommand("Setup", "Setup", typeof(Commands));
        public static readonly RoutedUICommand Plan = new RoutedUICommand("Plan", "Plan", typeof(Commands));
        public static readonly RoutedUICommand Replan = new RoutedUICommand("Replan", "Replan", typeof(Commands));
        public static readonly RoutedUICommand Results = new RoutedUICommand("Results", "Results", typeof(Commands));
        public static readonly RoutedUICommand Rebalance = new RoutedUICommand("Rebalance", "Rebalance", typeof(Commands));
        public static readonly RoutedUICommand Display = new RoutedUICommand("Display", "Display", typeof(Commands));
        public static readonly RoutedUICommand PlayPause = new RoutedUICommand("PlayPause", "PlayPause", typeof(Commands));
    }
}
