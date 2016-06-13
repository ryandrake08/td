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
            Session = session;
            DataContext = session.State;
        }

        private void PreviousRound_OnClick(object sender, RoutedEventArgs e)
        {
            var currentBlindLevel = Session.State["current_blind_level"];
            if (currentBlindLevel != 0)
            {
                Session.SetPreviousLevel(null);
            }
        }

        private void PauseResume_OnClick(object sender, RoutedEventArgs e)
        {
            var currentBlindLevel = Session.State["current_blind_level"];
            if (currentBlindLevel != 0)
            {
                Session.TogglePauseGame();
            }
            else
            {
                Session.StartGame();
            }
        }

        private void NextRound_OnClick(object sender, RoutedEventArgs e)
        {
            var currentBlindLevel = Session.State["current_blind_level"];
            if (currentBlindLevel != 0)
            {
                Session.SetNextLevel(null);
            }
        }

        private void CallClock_OnClick(object sender, RoutedEventArgs e)
        {
            var currentBlindLevel = Session.State["current_blind_level"];
            if (currentBlindLevel != 0)
            {
                var remaining = Session.State["action_clock_time_remaining"];
                if (remaining == 0)
                {
                    Session.SetActionClock(TournamentSession.ActionClockRequestTime);
                }
                else
                {
                    Session.ClearActionClock();
                }
            }
        }

        private TournamentSession Session { get; }
    }
}
