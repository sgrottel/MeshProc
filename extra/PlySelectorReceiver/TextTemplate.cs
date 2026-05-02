using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace PlySelectorReceiver
{
	internal class TextTemplate : INotifyPropertyChanged
	{
		private bool _selected = false;
		public bool Selected
		{
			get => _selected;
			set
			{
				if (_selected != value)
				{
					_selected = value;
					PropertyChanged?.Invoke(this, new(nameof(Selected)));
				}
			}
		}

		private string _name = string.Empty;
		public string Name
		{
			get => _name;
			set
			{
				if (_name != value)
				{
					_name = value;
					PropertyChanged?.Invoke(this, new(nameof(Name)));
				}
			}
		}
		private string _selType = string.Empty;
		public string SelectionType
		{
			get => _selType;
			set
			{
				if (_selType != value)
				{
					_selType = value;
					PropertyChanged?.Invoke(this, new(nameof(SelectionType)));
				}
			}
		}
		private string _text = string.Empty;
		public string Text
		{
			get => _text;
			set
			{
				if (_text != value)
				{
					_text = value;
					PropertyChanged?.Invoke(this, new(nameof(Text)));
				}
			}
		}

		public event PropertyChangedEventHandler? PropertyChanged;
	}
}
