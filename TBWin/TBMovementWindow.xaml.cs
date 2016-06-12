using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows;

namespace TBWin
{
    /// <summary>
    /// Interaction logic for TBMovementWindow.xaml
    /// </summary>
    public partial class TBMovementWindow : Window
    {
        public TBMovementWindow()
        {
            InitializeComponent();
        }

        public ObservableCollection<IDictionary<string, object>> Movements { get; set; } = new ObservableCollection<IDictionary<string, object>>();
    }
}
