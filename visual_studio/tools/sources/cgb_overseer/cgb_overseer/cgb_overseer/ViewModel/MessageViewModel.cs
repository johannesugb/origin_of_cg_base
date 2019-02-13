using cgb_overseer.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace cgb_overseer.ViewModel
{
	class MessageViewModel : BindableBase
	{
		public static MessageViewModel CreateSuccess(string text)
		{
			return new MessageViewModel
			{
				_messageColor = Brushes.DarkSeaGreen,
				_accentColor = Brushes.LightSeaGreen,
				_messageType = "GOOD",
				_messageText = text
			};
		}

		public static MessageViewModel CreateInfo(string text)
		{
			return new MessageViewModel
			{
				_messageColor = Brushes.LightSlateGray,
				_accentColor = Brushes.DimGray,
				_messageType = "INFO",
				_messageText = text
			};
		}

		public static MessageViewModel CreateWarning(string text)
		{
			return new MessageViewModel
			{
				_messageColor = Brushes.Orange,
				_accentColor = Brushes.DarkOrange,
				_messageType = "WARN",
				_messageText = text
			};
		}

		public static MessageViewModel CreateError(string text)
		{
			return new MessageViewModel
			{
				_messageColor = Brushes.LightCoral,
				_accentColor = Brushes.IndianRed,
				_messageType = "ERROR",
				_messageText = text
			};
		}

		public Brush MessageColor
		{
			get => _messageColor;
			set => SetProperty(ref _messageColor, value);
		}

		public Brush AccentColor
		{
			get => _accentColor;
			set => SetProperty(ref _accentColor, value);
		}

		public string MessageText
		{
			get => _messageText;
			set => SetProperty(ref _messageText, value);
		}

		public string MessageType
		{
			get => _messageType;
			set => SetProperty(ref _messageType, value);
		}

		private Brush _messageColor;
		private Brush _accentColor;
		private string _messageText;
		private string _messageType;
	}
}
