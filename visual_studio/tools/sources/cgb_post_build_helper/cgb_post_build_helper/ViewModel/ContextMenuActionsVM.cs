using CgbPostBuildHelper.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace CgbPostBuildHelper.ViewModel
{
	class ContextMenuActionsVM : BindableBase
	{
		private WpfApplication _application;

		public ContextMenuActionsVM(WpfApplication application)
		{
			_application = application;

			ShowInstances = new DelegateCommand(_ =>
			{
				var wnd = new View.InstancesList
				{
					DataContext = _application.AllInstances
				};
				wnd.Show();
			});

			ShowMessages = new DelegateCommand(_ =>
			{
				_application.ShowMessagesList();
			});

			ClearMessages = new DelegateCommand(_ =>
			{
				_application.ShowMessagesList();
			});

			OpenSettings = new DelegateCommand(_ =>
			{
				var wnd = new View.SettingsView
				{
					DataContext = new ViewModel.SettingsVM()
				};
				wnd.Show();
			});

			ExitApplicationCommand = new DelegateCommand(_ =>
			{
				Application.Current.Shutdown();
			});
		}

		public ICommand ShowInstances { get; set; }
		
		public ICommand ShowMessages { get; set; }

		public ICommand ClearMessages { get; set; }

		public ICommand OpenSettings { get; set; }

		public ICommand ExitApplicationCommand { get; set; }
	}
}
