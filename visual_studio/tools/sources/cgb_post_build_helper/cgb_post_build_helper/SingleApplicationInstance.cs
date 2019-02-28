using CgbPostBuildHelper.Utils;
using CgbPostBuildHelper.ViewModel;
using Microsoft.VisualBasic.ApplicationServices;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace CgbPostBuildHelper
{
	/// <summary>
	/// There is only one instance of this application... and VisualBasic manages that for us! Thank you, VisualBasic! 
	/// </summary>
	class SingleApplicationInstance : WindowsFormsApplicationBase
	{
		/// <summary>
		/// Single running instance of our WPF application
		/// </summary>
		private WpfApplication _wpfApp;

		public SingleApplicationInstance()
		{
			// Enable single instance behavior:
			IsSingleInstance = true;
		}

		/// <summary>
		/// Called when the first instance is created
		/// </summary>
		/// <param name="e">Contains command line parameters</param>
		/// <returns>A System.Boolean that indicates if the application should continue starting up.</returns>
		protected override bool OnStartup(StartupEventArgs e)
		{
			// Start the first instance of this app
			_wpfApp = new WpfApplication()
			{
				ShutdownMode = System.Windows.ShutdownMode.OnExplicitShutdown
			};
			if (e.CommandLine.Count > 0)
			{
				_wpfApp.Dispatcher.Invoke(() => 
				{
					try
					{
						var instanceParams = CgbUtils.ParseCommandLineArgs(e.CommandLine.ToArray());
						_wpfApp.HandleNewInvocation(instanceParams);
					}
					catch (Exception ex)
					{
						_wpfApp.AddToMessagesList(MessageVM.CreateError(null, ex.Message, null)); // TODO: Window with more info?
						_wpfApp.ShowMessagesList();
					}
				});
			}
			_wpfApp.Run();
			return false;
		}

		protected override void OnShutdown()
		{
			base.OnShutdown();
		}

		/// <summary>
		/// The application is already running, but we're receiving new command line parameters to be handled...
		/// </summary>
		/// <param name="e">Contains the command line parameters, passed by the invoker</param>
		protected override void OnStartupNextInstance(StartupNextInstanceEventArgs e)
		{
			if (e.CommandLine.Count > 0)
			{
				_wpfApp.Dispatcher.Invoke(() =>
				{
					try
					{
						var instanceParams = CgbUtils.ParseCommandLineArgs(e.CommandLine.ToArray());
						_wpfApp.HandleNewInvocation(instanceParams);
					}
					catch (Exception ex)
					{
						_wpfApp.AddToMessagesList(MessageVM.CreateError(null, ex.Message, null)); // TODO: Window with more info?
						_wpfApp.ShowMessagesList();
					}
				});
			}
		}
		
	}
}
