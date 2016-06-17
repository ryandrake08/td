using System.Windows;

namespace TBWin
{
    /// <summary>
    /// Interaction logic for TBPlayerWindow.xaml
    /// </summary>
    public partial class TBPlayerWindow
    {
        public TBPlayerWindow(TournamentSession session)
        {
            InitializeComponent();
            DataContext = session.State;
        }
    }
}
