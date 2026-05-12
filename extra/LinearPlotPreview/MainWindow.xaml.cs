using NLua;
using OpenTK.Graphics.ES20;
using ScottPlot;
using System.Globalization;
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

		private void SetErrorText(string error)
		{
			ErrorText.Text = error;
			ErrorText.Visibility = string.IsNullOrEmpty(error) ? Visibility.Collapsed : Visibility.Visible;
		}

		private void Window_Loaded(object sender, RoutedEventArgs e)
		{
			SetErrorText(string.Empty);
			if (!string.IsNullOrWhiteSpace(Code.Text))
			{
				ButtonPlot_Click(sender, e);
				WpfPlot1.Plot.Axes.AutoScale();
			}
		}

		private void ButtonPlot_Click(object sender, RoutedEventArgs? e)
		{
			try
			{
				int segStartCnt = int.Parse(StartSegmentsCount.Text, CultureInfo.InvariantCulture);
				double segTarLen = double.Parse(SegmentSubdivTargetLength.Text, CultureInfo.InvariantCulture);
				const double nearZero = 0.000001;

				if (segStartCnt < 1) throw new InvalidOperationException("StartSegmentsCount must be 1 or larger");
				if (segTarLen < nearZero) throw new InvalidOperationException("SegmentSubdivTargetLength too small; must be larger than zero");

				List<double> xValues = new();
				List<double> yValues = new();
				List<double> aValues = new();

				using (var lua = new Lua())
				{
					lua.DoString($"function evalLinearPlot(a)\n{Code.Text}\nend");

					double x, y, ea;
					double px, py;

					void Eval(double a)
					{
						var result = lua.GetFunction("evalLinearPlot").Call(a);
						x = (double)result[0];
						y = (double)result[1];
						ea = a;
					}
					void Add()
					{
						xValues.Add(x);
						yValues.Add(y);
						aValues.Add(ea);
					}
					double Dist()
					{
						double dx = px - x;
						double dy = py - y;
						return Math.Sqrt(dx * dx + dy * dy);
					}

					Eval(0);
					Add();
					double da = 1.0 / segStartCnt;
					for (int segI = 0; segI < segStartCnt; segI++)
					{
						px = x;
						py = y;

						Eval(da * (1 + segI));
						if (Dist() < nearZero)
						{
							continue;
						}
						Add();
					}

					bool refine = true;
					while (refine)
					{
						refine = false;

						for (int segI = xValues.Count - 2; segI >= 0; segI--)
						{
							px = xValues[segI];
							x = xValues[segI + 1];
							py = yValues[segI];
							y = yValues[segI + 1];
							if (Dist() > segTarLen)
							{
								double pea = aValues[segI];
								ea = aValues[segI + 1];

								Eval((pea + ea) * 0.5);

								xValues.Insert(segI + 1, x);
								yValues.Insert(segI + 1, y);
								aValues.Insert(segI + 1, ea);

								refine = true;
							}
						}

					}
				}

				WpfPlot1.Plot.Clear();
				WpfPlot1.Plot.Add.Scatter(xValues.ToArray(), yValues.ToArray());
				WpfPlot1.Refresh();

				SetErrorText(string.Empty);
			}
			catch (Exception ex)
			{
				SetErrorText(ex.ToString());
			}
		}

		private void Code_PreviewKeyDown(object sender, KeyEventArgs e)
		{
			if (e.Key == Key.Enter && (Keyboard.Modifiers & ModifierKeys.Control) != 0)
			{
				e.Handled = true;
				ButtonPlot_Click(sender, null);
			}

		}
	}
}
