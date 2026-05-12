using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace LinearPlotPreview
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public MainWindow()
		{
			InitializeComponent();
		}

		private void Window_Loaded(object sender, RoutedEventArgs e)
		{
			if (Code.Text.Length > 0)
			{
				ButtonPlot_Click(sender, e);
				WpfPlot1.Plot.Axes.AutoScale();
			}
		}

		private void ButtonPlot_Click(object sender, RoutedEventArgs e)
		{
			List<double> x = new();
			List<double> y = new();

			for (double a = 0.0; a < Math.PI * 2.0; a += 0.3)
			{
				x.Add(Math.Sin(a));
				y.Add(Math.Cos(a));
			}

			WpfPlot1.Plot.Add.Scatter(x.ToArray(), y.ToArray());
			WpfPlot1.Refresh();
		}
	}
}