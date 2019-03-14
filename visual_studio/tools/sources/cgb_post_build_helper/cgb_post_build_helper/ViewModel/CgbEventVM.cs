using CgbPostBuildHelper.Model;
using CgbPostBuildHelper.Utils;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace CgbPostBuildHelper.ViewModel
{
	/// <summary>
	/// What's the type/origin of a certain event?
	/// </summary>
	enum CgbEventType
	{
		/// <summary>
		/// Event originates from a (post-)build step invocation
		/// </summary>
		Build,
		/// <summary>
		/// Event originates from a file-update (like file watcher having noticed a change)
		/// </summary>
		Update
	}

	/// <summary>
	/// Data to a certain event. In most cases, this will contain some files 
	/// (except when there are no assets or shaders assigned to a project at all)
	/// For events with the type 'Update', there will usually be exactly one file assigned.
	/// </summary>
	class CgbEventVM : BindableBase
	{
		private readonly DateTime _createDate = DateTime.Now;
		private readonly CgbEventType _eventType;

		public CgbEventVM(CgbEventType type)
		{
			_eventType = type;
		}

		public DateTime CreateDate => _createDate;

		public CgbEventType Type => _eventType;

		public string TypeDescription
		{
			get
			{
				switch(_eventType)
				{
					case CgbEventType.Build:
						return "Buid-Event";
					case CgbEventType.Update:
						return "Update-Event";
				}
				return "?Unknown-Event?";
			}
		}

		public Brush EventColor
		{
			get
			{
				switch (_eventType)
				{
					case CgbEventType.Build:
						return View.Constants.InfoBrushDark;
					case CgbEventType.Update:
						{
							if ((from x in Files select x.Messages.ContainsMessagesOfType(MessageType.Error)).Any(x => x == true))
							{
								return View.Constants.ErrorBrushDark;
							}
							if ((from x in Files select x.Messages.ContainsMessagesOfType(MessageType.Warning)).Any(x => x == true))
							{
								return View.Constants.WarningBrushDark;
							}
							return View.Constants.SuccessBrushDark;
						}
				}
				return View.Constants.ErrorBrushLight; // That should not happen
			}
		}

		public ObservableCollection<FileDeploymentDataVM> Files { get; } = new ObservableCollection<FileDeploymentDataVM>();
	}
}
