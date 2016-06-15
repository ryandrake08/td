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

        private async void PreviousRound_OnClick(object sender, RoutedEventArgs e)
        {
            var currentBlindLevel = Session.State["current_blind_level"];
            if (currentBlindLevel != 0)
            {
                await Session.SetPreviousLevel(null);
            }
        }

        private async void PauseResume_OnClick(object sender, RoutedEventArgs e)
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

        private async void NextRound_OnClick(object sender, RoutedEventArgs e)
        {
            var currentBlindLevel = Session.State["current_blind_level"];
            if (currentBlindLevel != 0)
            {
                await Session.SetNextLevel(null);
            }
        }

        private async void CallClock_OnClick(object sender, RoutedEventArgs e)
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

        private TournamentSession Session { get; }
    }
}
