using System;
using System.Windows;

namespace TBWin
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public sealed partial class App : Application, IDisposable
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

            if (document == null)
            {
                _mainWindow.Document = new JsonDocument();
            }
            else
            {
                _mainWindow.Document = new JsonDocument(document);
            }

            _mainWindow.Show();
        }

        public void Dispose()
        {
            ((IDisposable)_mainWindow).Dispose();
        }

        private MainWindow _mainWindow;
    }
}
