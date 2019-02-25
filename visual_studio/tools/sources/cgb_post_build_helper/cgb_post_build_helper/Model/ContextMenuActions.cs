using CgbPostBuildHelper.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace CgbPostBuildHelper.Model
{
	class ContextMenuActions : BindableBase
	{
		private WpfApplication _application;

		public ContextMenuActions(WpfApplication application)
		{
			_application = application;
			
			ShowWindowCommand = new DelegateCommand(_ =>
			{
				_application.ShowMessagesList();
			});

			HideWindowCommand = new DelegateCommand(_ =>
			{
				_application.CloseMessagesListLater(false);
			});

			ExitApplicationCommand = new DelegateCommand(_ =>
			{
				Application.Current.Shutdown();
			});
		}

		public ICommand ShowWindowCommand { get; set; }
		
		public ICommand HideWindowCommand { get; set; }

		public ICommand ExitApplicationCommand { get; set; }
	}
}
