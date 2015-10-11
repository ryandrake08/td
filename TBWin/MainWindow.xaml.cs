using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace TBWin
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            _daemon = new TournamentDaemon();
            _daemon.Start(0);
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

        private void OpenCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            _document.Open();
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

        private JsonDocument _document;
        private TournamentDaemon _daemon;
    }

    public static class Commands
    {
        public static readonly RoutedUICommand Exit = new RoutedUICommand("Exit", "Exit", typeof(Commands), new InputGestureCollection() { new KeyGesture(Key.F4, ModifierKeys.Alt) });
    }
}
