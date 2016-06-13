using System.Windows;

namespace TBWin
{
    /// <summary>
    /// Interaction logic for TBPlanWindow.xaml
    /// </summary>
    public partial class TBPlanWindow
    {
        public TBPlanWindow()
        {
            InitializeComponent();
        }

        private void OKButton_OnClick(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }
    }
}
