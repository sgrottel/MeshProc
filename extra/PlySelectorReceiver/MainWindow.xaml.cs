using Microsoft.Win32;
using System.Collections.ObjectModel;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;

namespace PlySelectorReceiver
{

	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		private ObservableCollection<SelectionData> selections = new();
		private ObservableCollection<TextTemplate> templates = new();

		public MainWindow()
		{
			InitializeComponent();
			ReceivedSelectionsList.ItemsSource = selections;
			TemplateList.ItemsSource = templates;
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
				if (!Clipboard.ContainsText())
				{
					StatusText.Text = "Error: PlySelector Export signaled, but clipboard had no text";
					return IntPtr.Zero;
				}

				string text = Clipboard.GetText();
				if (string.IsNullOrEmpty(text))
				{
					StatusText.Text = "Error: PlySelector Export signaled, but sent text was empty";
					return IntPtr.Zero;
				}

				SelectionData? newData;
				try
				{
					newData = JsonSerializer.Deserialize<SelectionData>(text);
					if (newData == null)
					{
						StatusText.Text = "Error: PlySelector Export signaled, but failed to parse sent json";
						return IntPtr.Zero;
					}
				}
				catch (Exception ex)
				{
					StatusText.Text = $"Error: PlySelector Export signaled, but failed to parse sent json; {ex}";
					return IntPtr.Zero;
				}

				if (!newData.IsValid())
				{
					StatusText.Text = $"Error: PlySelector Export, but new data is not valid";
					return IntPtr.Zero;
				}

				selections.Add(newData);
				ReceivedSelectionsList.SelectedItem = newData;
				StatusText.Text = "Selection received";
				Compile(newData);

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
				Compile(sel);
			}
		}

		private void Compile(SelectionData sel)
		{
			// throw new NotImplementedException();
		}

		private void ButtonCopyResultsText_Click(object sender, RoutedEventArgs e)
		{
			Clipboard.SetText(
				(ResultsText.SelectionLength > 0)
				? ResultsText.SelectedText
				: ResultsText.Text);
			StatusText.Text = "Results text copied to clipboard";
		}

		private void ContentControlStatusText_MouseDoubleClick(object sender, MouseButtonEventArgs e)
		{
			StatusText.TextWrapping =
				(StatusText.TextWrapping == TextWrapping.NoWrap)
				? TextWrapping.Wrap : TextWrapping.NoWrap;
		}

		private string SelectionDataFile { get; } = System.IO.Path.Join(AppContext.BaseDirectory, "selection_data.json");

		private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			File.WriteAllText(
				path: SelectionDataFile,
				contents: JsonSerializer.Serialize(selections.ToArray()),
				encoding: new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
		}

		private void Window_Loaded(object sender, RoutedEventArgs e)
		{
			LoadTemplatesButton_Click(sender, e);
			ButtonReloadLastSelections_Click(sender, e);
		}

		private void ButtonReloadLastSelections_Click(object sender, RoutedEventArgs e)
		{
			if (File.Exists(SelectionDataFile))
			{
				try
				{
					LoadSelectionData(SelectionDataFile);
				}
				catch (Exception ex)
				{
					StatusText.Text = $"'selection_data.json' file exists but cannot be loaded; {ex}";
				}
			}
		}

		private void LoadSelectionData(string filename)
		{
			SelectionData[]? selectionData = JsonSerializer.Deserialize<SelectionData[]>(File.ReadAllText(filename));
			if (selectionData == null)
			{
				throw new InvalidOperationException("Deserializing returned null");
			}
			selections.Clear();
			foreach (var s in selectionData)
			{
				selections.Add(s);
			}
		}

		private void ButtonDeleteSelectedSelections_Click(object sender, RoutedEventArgs e)
		{
			foreach (SelectionData sel in ReceivedSelectionsList.SelectedItems.Cast<SelectionData>().ToArray())
			{
				selections.Remove(sel);
			}
		}

		private void ButtonLoadSelectionsData_Click(object sender, RoutedEventArgs e)
		{
			OpenFileDialog openFileDialog = new OpenFileDialog()
			{
				Filter = "Json files (*.json)|*.json|All files (*.*)|*.*"
			};
			if (openFileDialog.ShowDialog() == true)
			{
				try
				{
					LoadSelectionData(openFileDialog.FileName);
					StatusText.Text = $"Loaded data from: {openFileDialog.FileName}";
				}
				catch (Exception ex)
				{
					StatusText.Text = $"Failed to load data from \"{openFileDialog.FileName}\": {ex}";
				}
			}
		}

		private void ButtonSaveSelectionsData_Click(object sender, RoutedEventArgs e)
		{
			SaveFileDialog saveFileDialog = new SaveFileDialog()
			{
				Filter = "Json file (*.json)|*.json"
			};
			if (saveFileDialog.ShowDialog() == true)
			{
				try
				{
					File.WriteAllText(
						path: saveFileDialog.FileName,
						contents: JsonSerializer.Serialize(selections.ToArray()),
						encoding: new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
					StatusText.Text = $"Saved data to: {saveFileDialog.FileName}";
				}
				catch(Exception ex)
				{
					StatusText.Text = $"Failed to save data to \"{saveFileDialog.FileName}\": {ex}";
				}
			}
		}

		private void AddTemplateButton_Click(object sender, RoutedEventArgs e)
		{
			TextTemplate tt = new() { Name = "New Template", SelectionType = "plane" };
			if (templates.Count > 0)
			{
				tt.Name += $" {templates.Count + 1}";
			}
			templates.Add(tt);
			TemplateList.SelectedItem = tt;
		}

		private void DeleteTemplatesButton_Click(object sender, RoutedEventArgs e)
		{
			var si = TemplateList.SelectedIndex;
			foreach (var tt in TemplateList.SelectedItems.Cast<TextTemplate>().ToArray())
			{
				templates.Remove(tt);
			}
			si = Math.Min(si, templates.Count - 1);
			if (si >= 0)
			{
				TemplateList.SelectedIndex = si;
			}
		}

		private string templatesFile = Path.Combine(AppContext.BaseDirectory, "templates.json");

		private void SaveTemplatesButton_Click(object sender, RoutedEventArgs e)
		{
			File.WriteAllText(
				path: templatesFile,
				contents: JsonSerializer.Serialize<TextTemplate[]>(templates.ToArray(), options: new JsonSerializerOptions() { WriteIndented = true }),
				encoding: new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
		}

		private void LoadTemplatesButton_Click(object sender, RoutedEventArgs e)
		{
			string? d = Path.GetDirectoryName(templatesFile) ?? AppContext.BaseDirectory;
			string fn = Path.GetFileName(templatesFile);
			while (d != null && !File.Exists(Path.Combine(d, fn)))
			{
				d = Path.GetDirectoryName(d);
			}
			if (d == null)
			{
				StatusText.Text = "Failed to find templates file";
				return;
			}

			TextTemplate[]? tts = JsonSerializer.Deserialize<TextTemplate[]>(File.ReadAllText(Path.Combine(d, fn)));
			if (tts == null)
			{
				StatusText.Text = "Failed to read templates file";
				return;
			}

			templates.Clear();
			foreach (var tt in tts)
			{
				templates.Add(tt);
			}
			templatesFile = Path.Combine(d, fn);
		}

		private TextTemplate? _previousSelectedTemplate;

		private void TemplateList_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
		{
			TextTemplate? sel = TemplateList.SelectedItem as TextTemplate;
			if (sel == _previousSelectedTemplate)
			{
				return;
			}
			if (sel == null)
			{
				TemplatePreview.DataContext = new TextTemplate();
				_previousSelectedTemplate = null;
			}
			else
			{
				TemplatePreview.DataContext = JsonSerializer.Deserialize<TextTemplate>(JsonSerializer.Serialize(sel)) ?? new TextTemplate();
				_previousSelectedTemplate = sel;
			}
		}

		private void StoreTemplateButton_Click(object sender, RoutedEventArgs e)
		{
			if (_previousSelectedTemplate == null)
			{
				StatusText.Text = "Cannot store template values, as the list entry is not found";
				return;
			}
			TextTemplate? tt = TemplatePreview.DataContext as TextTemplate;
			if (tt == null)
			{
				StatusText.Text = "Cannot store template values, as the working copy seems invalid";
				return;
			}
			_previousSelectedTemplate.Name = tt.Name;
			_previousSelectedTemplate.SelectionType = tt.SelectionType;
			_previousSelectedTemplate.Text = tt.Text;
		}
	}
}
