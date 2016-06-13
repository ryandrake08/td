using System.Windows;

namespace TBWin
{
    /// <summary>
    /// Interaction logic for TBAuthCodeWindow.xaml
    /// </summary>
    public partial class TBAuthCodeWindow
    {
        public TBAuthCodeWindow()
        {
            InitializeComponent();
        }

        private void OKButton_OnClick(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }
    }
}
