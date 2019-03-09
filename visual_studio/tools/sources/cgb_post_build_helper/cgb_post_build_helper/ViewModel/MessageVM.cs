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

	class MessageVM : BindableBase
	{
		private readonly CgbAppInstanceVM _instance;
		private ICommand _additionalInfoCmd;
		private Model.Message _model;

		public MessageVM(CgbAppInstanceVM instance, Model.Message model)
		{
			_instance = instance;
			_model = model;
		}

		public DateTime CreateDate => _model.CreateDate;

		public CgbAppInstanceVM AppInstance => _instance;

		public string AppInstancePath => _instance.Path;

		public string AppInstanceName => new FileInfo(AppInstancePath).Name;

		public Brush MessageColor
		{
			get
			{
				switch (_model.MessageType)
				{
					case MessageType.Success:
						return View.Constants.SuccessBrushLight;
					case MessageType.Information:
						return View.Constants.InfoBrushLight;
					case MessageType.Warning:
						return View.Constants.WarningBrushLight;
					case MessageType.Error:
						return View.Constants.ErrorBrushLight;
					default:
						return View.Constants.InfoBrushLight;
				}
			}
		}

		public Brush AccentColor
		{
			get
			{
				switch (_model.MessageType)
				{
					case MessageType.Success:
						return View.Constants.SuccessBrushDark;
					case MessageType.Information:
						return View.Constants.InfoBrushDark;
					case MessageType.Warning:
						return View.Constants.WarningBrushDark;
					case MessageType.Error:
						return View.Constants.ErrorBrushDark;
					default:
						return View.Constants.InfoBrushDark;
				}
			}
		}

		public string MessageText
		{
			get => _model.MessageText;
			set => SetProperty(_model, m => m.MessageText, value);
		}

		public MessageType MessageType
		{
			get => _model.MessageType;
			set 
			{
				SetProperty(_model, m => m.MessageType, value);
				OnPropertyChanged("MessageTypeDescription");
				OnPropertyChanged("MessageTypeShortDescription");
			}
		}

		public string MessageTypeDescription
		{
			get
			{
				switch (_model.MessageType)
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
				switch (_model.MessageType)
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
			get => _model.Action == null ? null : new DelegateCommand(_ => _model.Action());
		}
	}
}
