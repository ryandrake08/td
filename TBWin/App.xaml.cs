using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

namespace TBWin
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
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

        private MainWindow _mainWindow;
    }
}
