using System.Collections.ObjectModel;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace PlySelectorReceiver
{

	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		private ObservableCollection<SelectionData> selections = new();

		public MainWindow()
		{
			InitializeComponent();

			ReceivedSelectionsList.ItemsSource = selections;
		}

		#region Receiving SGR_PLYSELECTOR_EXPORT

		[DllImport("user32.dll", CharSet = CharSet.Unicode)]
		static extern uint RegisterWindowMessage(string lpString);

		public static readonly uint SgrPlySelectorExport = RegisterWindowMessage("SGR_PLYSELECTOR_EXPORT");

		private HwndSource? _source;

		protected override void OnSourceInitialized(EventArgs e)
		{
			base.OnSourceInitialized(e);

			_source = (HwndSource)PresentationSource.FromVisual(this);
			_source.AddHook(WndProc);
		}

		private IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
		{
			if ((uint)msg == SgrPlySelectorExport)
			{
				SelectionData no = new();
				selections.Add(no);
				ReceivedSelectionsList.SelectedItem = no;

				handled = true;
			}
			return IntPtr.Zero;
		}

		#endregion

		private void ButtonClearReceivedSelectionsList_Click(object sender, RoutedEventArgs e)
		{
			selections.Clear();
		}

		private void ButtonClearResultsText_Click(object sender, RoutedEventArgs e)
		{
			ResultsText.Clear();
		}

		private void ButtonCompileSelectedReceivedSelection_Click(object sender, RoutedEventArgs e)
		{
			foreach (var si in ReceivedSelectionsList.SelectedItems)
			{
				SelectionData? sel = si as SelectionData;
				if (sel == null) continue;



			}
		}

		private void ButtonCopyResultsText_Click(object sender, RoutedEventArgs e)
		{
			Clipboard.SetText(
				(ResultsText.SelectionLength > 0)
				? ResultsText.SelectedText
				: ResultsText.Text);
		}
	}
}
