using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.IO;
using Hardcodet.Wpf.TaskbarNotification;
using cgb_overseer.Model;
using cgb_overseer.Utils;
using System.Text.RegularExpressions;
using cgb_overseer.View;
using cgb_overseer.ViewModel;

namespace cgb_overseer
{
	/// <summary>
	/// The only instance of our application which SingleApplicationInstance interfaces with
	/// in order to start new actions, instruct file watches, etc.
	/// </summary>
	class WpfApplication : Application
	{
		static readonly Regex RegexFilterEntry = new Regex(@"<Object\s+.*?Include\s*?\=\s*?\""(.*?)\""\s*?\>.*?\<Filter\s*?.*?\>(.*?)\<\/Filter\>.*?\<\/Object\>", 
			RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.IgnoreCase);

		/// <summary>
		/// Single instance of our TaskbarIcon
		/// </summary>
		private TaskbarIcon _taskbarIcon;

		public WpfApplication()
		{
			Stream iconStream = System.Windows.Application.GetResourceStream(new Uri("pack://application:,,,/tray_icon.ico")).Stream;
			_taskbarIcon = new TaskbarIcon()
			{
				Icon = new System.Drawing.Icon(iconStream),
				ToolTipText = "Hello Tray"
			};
		}

		protected override void OnStartup(StartupEventArgs e)
		{
			base.OnStartup(e);
		}

		/// <summary>
		/// The single application instance is getting teared down => cleanup!
		/// </summary>
		protected override void OnExit(ExitEventArgs e)
		{
			_taskbarIcon.Dispose();
			base.OnExit(e);
		}

		/// <summary>
		/// Handle a new invocation (usually triggerd by a post build step out of VisualStudio)
		/// </summary>
		/// <param name="p">All the parameters passed by that invocation/post build step</param>
		public void HandleNewInvocation(InvocationParams p)
		{
			// 1. Parse the .filters file for asset files and shader files

			// 2. Determine dependencies of those files

			// 3. Deploy all the original files and their dependencies
			// 3.1 If we have to, compile them to spir-v
		}
		
		public void ShowBalloonMessage(MessageViewModel mvm)
		{
			var vm = new MessagesListViewModel();
			vm.Items.Add(mvm);
			vm.Items.Add(MessageViewModel.CreateInfo("a"));
			vm.Items.Add(MessageViewModel.CreateWarning("c"));
			vm.Items.Add(MessageViewModel.CreateSuccess("d"));
			vm.Items.Add(MessageViewModel.CreateError("e"));
			var view = new MessagesList()
			{
				DataContext = vm
			};
			_taskbarIcon.ShowCustomBalloon(view, System.Windows.Controls.Primitives.PopupAnimation.Fade, 4000);
		}
	}
}
