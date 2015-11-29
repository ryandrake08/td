using System;
using System.Windows;

namespace TBWin
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public sealed partial class App : IDisposable
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            _mainWindow = new MainWindow();
            string document = null;

            if (e.Args.Length > 0)
            {
                // Parse the command-line arguments
                foreach (string arg in e.Args)
                {
                    if (arg[0] != '-')
                    {
                        document = arg;
                    }
                }
            }

            _mainWindow.Document = document == null ? new JsonDocument() : new JsonDocument(document);
            _mainWindow.Show();
        }

        public void Dispose()
        {
            _mainWindow.Dispose();
        }

        private MainWindow _mainWindow;
    }
}
