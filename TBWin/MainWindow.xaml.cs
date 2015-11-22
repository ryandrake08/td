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
    public sealed partial class MainWindow : Window, IDisposable
    {
        public MainWindow()
        {
            InitializeComponent();

            _daemon = new TournamentDaemon();
            int port = _daemon.Start(TournamentSession.ClientIdentifier());
            _daemon.Publish("TBWin");
            _session = new TournamentSession();
            _connectTask = _session.Connect(IPAddress.Loopback, port);
        }

        public JsonDocument Document
        {
            get { return _document; }
            set { _document = value; }
        }

        private void NewCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            _document = new JsonDocument();
        }

        private async void OpenCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            _document.Open();

            await _session.Configure(_document.Content, delegate (IDictionary<string, dynamic> newConfig)
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

        public void Dispose()
        {
            _daemon.Dispose();
            _session.Dispose();
        }

        private JsonDocument _document;
        private TournamentDaemon _daemon;
        private TournamentSession _session;
        private Task _connectTask;
    }

    public static class Commands
    {
        public static readonly RoutedUICommand Exit = new RoutedUICommand("Exit", "Exit", typeof(Commands), new InputGestureCollection() { new KeyGesture(Key.F4, ModifierKeys.Alt) });
    }
}
