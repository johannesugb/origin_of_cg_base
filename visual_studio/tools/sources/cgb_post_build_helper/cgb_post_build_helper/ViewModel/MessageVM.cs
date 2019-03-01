using CgbPostBuildHelper.Model;
using CgbPostBuildHelper.Utils;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Windows.Media;

namespace CgbPostBuildHelper.ViewModel
{
	enum MessageType
	{
		Success,
		Information,
		Warning,
		Error,
	}

	class MessageVM : BindableBase
	{
		#region static factory methods
		public static MessageVM CreateSuccess(CgbAppInstance appInstance, string text, ICommand additionalInfoCommand)
		{
			return new MessageVM(appInstance)
			{
				_messageColor = View.Constants.SuccessBrushLight,
				_accentColor = View.Constants.SuccessBrushDark,
				_messageType = MessageType.Success,
				_messageText = text,
				_additionalInfoCmd = additionalInfoCommand
			};
		}

		public static MessageVM CreateInfo(CgbAppInstance appInstance, string text, ICommand additionalInfoCommand)
		{
			return new MessageVM(appInstance)
			{
				_messageColor = View.Constants.InfoBrushLight,
				_accentColor = View.Constants.InfoBrushDark,
				_messageType = MessageType.Information,
				_messageText = text,
				_additionalInfoCmd = additionalInfoCommand
			};
		}

		public static MessageVM CreateWarning(CgbAppInstance appInstance, string text, ICommand additionalInfoCommand)
		{
			return new MessageVM(appInstance)
			{
				_messageColor = View.Constants.WarningBrushLight,
				_accentColor = View.Constants.WarningBrushDark,
				_messageType = MessageType.Warning,
				_messageText = text,
				_additionalInfoCmd = additionalInfoCommand
			};
		}

		public static MessageVM CreateError(CgbAppInstance appInstance, string text, ICommand additionalInfoCommand)
		{
			return new MessageVM(appInstance)
			{
				_messageColor = View.Constants.ErrorBrushLight,
				_accentColor = View.Constants.ErrorBrushDark,
				_messageType = MessageType.Error,
				_messageText = text,
				_additionalInfoCmd = additionalInfoCommand
			};
		}
		#endregion

		private readonly DateTime _createDate = DateTime.Now;
		private readonly CgbAppInstance _instance;
		private Brush _messageColor;
		private Brush _accentColor;
		private string _messageText;
		private MessageType _messageType;
		private ICommand _additionalInfoCmd;

		public MessageVM(CgbAppInstance instance)
		{
			_instance = instance;
		}

		public DateTime CreateDate => _createDate;

		public CgbAppInstance AppInstance => _instance;

		public string AppInstancePath => _instance.Path;

		public string AppInstanceName => new FileInfo(AppInstancePath).Name;

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

		public MessageType MessageType
		{
			get => _messageType;
			set 
			{
				SetProperty(ref _messageType, value);
				OnPropertyChanged("MessageTypeDescription");
				OnPropertyChanged("MessageTypeShortDescription");
			}
		}

		public string MessageTypeDescription
		{
			get
			{
				switch (_messageType)
				{
					case MessageType.Success:
						return "Operation Succeeded";
					case MessageType.Information:
						return "Information";
					case MessageType.Warning:
						return "Warning";
					case MessageType.Error:
						return "Operation Failed/Error";
					default:
						return "???????";
				}
			}
		}

		public string MessageTypeShortDescription
		{
			get
			{
				switch (_messageType)
				{
					case MessageType.Success:
						return "DONE";
					case MessageType.Information:
						return "INFO";
					case MessageType.Warning:
						return "WARN";
					case MessageType.Error:
						return "ERROR";
					default:
						return "????";
				}
			}
		}

		public ICommand AdditionalInfoCmd
		{
			get => _additionalInfoCmd;
			set => SetProperty(ref _additionalInfoCmd, value);
		}
	}
}
