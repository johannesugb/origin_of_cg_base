using post_build_helper.Utils;
using System;
using System.Windows;
using System.Windows.Input;

namespace post_build_helper.ViewModel
{
	public class NotifyIconViewModel
	{
		/// <summary>
		/// Shows a window, if none is already open.
		/// </summary>
		public ICommand ShowWindowCommand
		{
			get
			{
				return new DelegateCommand(
					_ =>
					{
						Application.Current.MainWindow = new MainWindow();
						Application.Current.MainWindow.Show();
					}, 
					_ => Application.Current.MainWindow == null);
			}
		}

		/// <summary>
		/// Hides the main window. This command is only enabled if a window is open.
		/// </summary>
		public ICommand HideWindowCommand
		{
			get
			{
				return new DelegateCommand(
					_ => Application.Current.MainWindow.Close(),
					_ => Application.Current.MainWindow != null);
			}
		}


		/// <summary>
		/// Shuts down the application.
		/// </summary>
		public ICommand ExitApplicationCommand
		{
			get
			{
				return new DelegateCommand(
					_ => Application.Current.Shutdown());
			}
		}
	}
}
