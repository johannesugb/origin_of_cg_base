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
using System.Windows.Threading;

namespace cgb_overseer
{
	/// <summary>
	/// The only instance of our application which SingleApplicationInstance interfaces with
	/// in order to start new actions, instruct file watches, etc.
	/// </summary>
	class WpfApplication : Application, IMessageListLifetimeHandler
	{
		static readonly Regex RegexFilterEntry = new Regex(@"<Object\s+.*?Include\s*?\=\s*?\""(.*?)\""\s*?\>.*?\<Filter\s*?.*?\>(.*?)\<\/Filter\>.*?\<\/Object\>", 
			RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.IgnoreCase);

		/// <summary>
		/// Single instance of our TaskbarIcon
		/// </summary>
		private TaskbarIcon _taskbarIcon;
		private DateTime _messageListAliveUntil;
		private static readonly TimeSpan MessageListCloseDelay = TimeSpan.FromSeconds(2.0);
		private readonly MessagesListViewModel _messagesListVM = new MessagesListViewModel();
		private readonly MessagesList _messagesListView;

		/// <summary>
		/// This one is important: 
		/// It stores all asset files associated with one build target. Upon HandleNewInvocation, the 
		/// file list is updated and those assets which HAVE CHANGED (based on a file-hash) are copied 
		/// to their respective target paths.
		/// 
		/// The keys of this dictionary are target paths to built executables.
		/// 
		/// The Overseer will listen for process starts of those files, referenced in the keys. 
		/// In case, a process' full path name matches one of those keys, the file watcher for all its 
		/// child files will be launched.
		/// </summary>
		private readonly Dictionary<string, List<AssetFile>> _assetFilesPerTarget;

		public WpfApplication()
		{
			 _messagesListView = new MessagesList()
			 {
				 LifetimeHandler = this,
				 DataContext = _messagesListVM
			 };

			Stream iconStream = System.Windows.Application.GetResourceStream(new Uri("pack://application:,,,/tray_icon.ico")).Stream;
			_taskbarIcon = new TaskbarIcon()
			{
				Icon = new System.Drawing.Icon(iconStream),
				ToolTipText = "Hello Tray"
			};
		}

		private void DispatcherInvokeLater(TimeSpan delay, Action action)
		{
			var timer = new DispatcherTimer { Interval = delay };
			timer.Start();
			timer.Tick += (sender, args) =>
			{
				timer.Stop();
				action();
			};
		}

		private void CloseMessagesListLater(bool setAliveTime)
		{
			if (setAliveTime)
			{
				_messageListAliveUntil = DateTime.Now + MessageListCloseDelay;
			}

			DispatcherInvokeLater(MessageListCloseDelay, () =>
			{
				if (_messageListAliveUntil <= DateTime.Now)
				{
					_taskbarIcon.CloseBalloon();
				}
				else if (_messageListAliveUntil != DateTime.MaxValue)
				{
					CloseMessagesListLater(false);
				}
			});
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
			// See, if we're already handling that executable!
			List<AssetFile> newAssetsList;
			List<AssetFile> assetsListToUpdate;
			if (_assetFilesPerTarget.ContainsKey(p.ExecutablePath))
			{
				assetsListToUpdate = _assetFilesPerTarget[p.ExecutablePath];
				newAssetsList = null;
			}
			else // executable path not included in the dictionary
			{
				assetsListToUpdate = null;
				newAssetsList = new List<AssetFile>();
			}

			// TODO: PRoceed here (files durchgehen und so, extension methods in die CgbUtils klasse usw.)

			// 1. Parse the .filters file for asset files and shader files
			{ 
				var filtersContent = File.ReadAllText(p.FiltersPath);
				var filters = RegexFilterEntry.Matches(filtersContent);
				foreach (Match match in filters)
				{
					var fileInQuestion = new AssetFile
					{
						
					};
					// TODO: use this method (which is still to be implemented):
					newAssetsList?.AddOrReplaceAssetFile(match.Groups[1].Value, match.Groups[2].Value);
					assetsListToUpdate?.AddOrReplaceAssetFile(match.Groups[1].Value, match.Groups[2].Value);
				}
			}

			// 2. Determine dependencies of those files

			// 3. Deploy all the original files and their dependencies
			// 3.1 If we have to, compile them to spir-v
		}

		public void AddToAndShowMessagesList(MessageViewModel mvm)
		{
			_messagesListVM.Items.Add(mvm);
			_taskbarIcon.ShowCustomBalloon(_messagesListView, System.Windows.Controls.Primitives.PopupAnimation.None, null);
			CloseMessagesListLater(true);
		}

		public void KeepAlivePermanently()
		{
			_messageListAliveUntil = DateTime.MaxValue;
		}

		public void FadeOutOrBasicallyDoWhatYouWantIDontCareAnymore()
		{
			CloseMessagesListLater(true);
		}
	}
}
