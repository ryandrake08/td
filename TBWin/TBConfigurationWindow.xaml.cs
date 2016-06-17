using System;
using System.Windows;

namespace TBWin
{
    /// <summary>
    /// Interaction logic for TBConfigurationWindow.xaml
    /// </summary>
    public partial class TBConfigurationWindow
    {
        public TBConfigurationWindow(JsonDocument document)
        {
            InitializeComponent();
            Document = document;
            DataContext = document.Content;
        }

        private void AddPlayer_OnClick(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void RemovePlayer_OnClick(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void AddChip_Click(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void RemoveChip_Click(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void AddFunding_Click(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void RemoveFunding_Click(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void AddRound_Click(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void RemoveRound_Click(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void AddDevice_Click(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void RemoveDevice_Click(object sender, RoutedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void OKButton_OnClick(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }

        private JsonDocument Document { get; }
    }
}
