using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.IO;
using Hardcodet.Wpf.TaskbarNotification;
using CgbPostBuildHelper.Model;
using CgbPostBuildHelper.Utils;
using System.Text.RegularExpressions;
using CgbPostBuildHelper.View;
using CgbPostBuildHelper.ViewModel;
using System.Windows.Threading;
using System.Windows.Controls;

namespace CgbPostBuildHelper
{
	/// <summary>
	/// The only instance of our application which SingleApplicationInstance interfaces with
	/// in order to start new actions, instruct file watches, etc.
	/// </summary>
	class WpfApplication : Application, IMessageListLifetimeHandler
	{
		static readonly Regex RegexFilterEntry = new Regex(@"<(None|Object|Image)\s+.*?Include\s*?\=\s*?\""(.*?)\""\s*?\>.*?\<Filter\s*?.*?\>(.*?)\<\/Filter\>.*?\<\/\1\>", 
			RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.IgnoreCase);

		/// <summary>
		/// Single instance of our TaskbarIcon
		/// </summary>
		private TaskbarIcon _taskbarIcon;
		private DateTime _messageListAliveUntil;
		private static readonly TimeSpan MessageListCloseDelay = TimeSpan.FromSeconds(2.0);
		private readonly MessagesListVM _messagesListVM = new MessagesListVM();
		private readonly MessagesList _messagesListView;

		/// <summary>
		/// This one is important: 
		/// It stores all asset files associated with one build target. Upon HandleNewInvocation, the 
		/// file list is updated and those assets which HAVE CHANGED (based on a file-hash) are copied 
		/// to their respective target paths.
		/// 
		/// The Overseer will listen for process starts of those files, referenced in the keys. 
		/// In case, a process' full path name matches one of those keys, the file watcher for all its 
		/// child files will be launched.
		/// </summary>
		private readonly List<CgbAppInstance> _instances = new List<CgbAppInstance>();

		public WpfApplication()
		{
			_messagesListView = new MessagesList()
			{
				 LifetimeHandler = this,
				 DataContext = _messagesListVM
			};

			Stream iconStream = System.Windows.Application.GetResourceStream(new Uri("pack://application:,,,/tray_icon.ico")).Stream;
			
			var rd = new ResourceDictionary()
			{
				Source = new Uri(";component/ContextMenuResources.xaml", UriKind.RelativeOrAbsolute)
			};

			_taskbarIcon = new TaskbarIcon()
			{
				Icon = new System.Drawing.Icon(iconStream),
				ToolTipText = "CGB Post Build Helper",
				ContextMenu = (ContextMenu)rd["SysTrayMenu"]
			};
			_taskbarIcon.ContextMenu.DataContext = new ContextMenuActions(this);
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

		public void CloseMessagesListLater(bool setAliveTime)
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
			var inst = _instances.GetInstance(p.ExecutablePath);
			var prevAssetsList = inst?.Files; // null if there is no previous instance

			// Create new or update config/invocation params:
			if (null == inst)
			{
				inst = new CgbAppInstance
				{
					Config = p
				};
				_instances.Add(inst);
			}
			else
			{
				inst.Config = p;
				// Proceed with an empty list because files could have changed:
				inst.Files.Clear();
			}

			// Parse the .filters file for asset files and shader files
			{ 
				var filtersFile = new FileInfo(p.FiltersPath);
				var filtersContent = File.ReadAllText(filtersFile.FullName);
				var filters = RegexFilterEntry.Matches(filtersContent);

				var hasErrors = false;
				var hasWarnings = false;
				var nFilesDeployed = 0;
				var mustShowMessages = false;

				// -> Store the whole event (but a little bit later)
				var cgbEvent = new CgbEventVM(CgbEventType.Build);
				
				// -> Parse the .filters file and deploy each and every file
				foreach (Match match in filters)
				{
					var filePath = Path.Combine(filtersFile.DirectoryName, match.Groups[2].Value);
					var filterPath = match.Groups[3].Value;

					try
					{
						inst.DeployFile(
							prevAssetsList, 
							filePath, filterPath, 
							out var deployedFiles);

						foreach (var deployedFile in deployedFiles)
						{
							// For the current files list:
							inst.Files.Add(deployedFile);

							hasErrors = hasErrors || deployedFile.Messages.ContainsMessagesOfType(MessageType.Error);
							hasWarnings = hasWarnings || deployedFile.Messages.ContainsMessagesOfType(MessageType.Warning);

							// For the event:
							cgbEvent.Files.Add(deployedFile);
						}
						nFilesDeployed += deployedFiles.Count;
					}
					catch (Exception ex)
					{
						AddToMessagesList(MessageVM.CreateError(inst, ex.Message, null)); // TODO: perform some action
						mustShowMessages = true;
						continue;
					}
				}

				// Now, store the event
				inst.AllEvents.Add(cgbEvent);

				// Analyze the outcome, create a message and possibly show it, if there could be a problem (files with warnings or errors)
				if (hasErrors || hasWarnings)
				{
					if (hasWarnings && hasErrors)
						AddToMessagesList(MessageVM.CreateError(inst, $"Deployed {nFilesDeployed} with ERRORS and WARNINGS.", new DelegateCommand(_ =>
						{
							Window window = new Window
							{
								Title = "Build-Event Details including ERRORS and WARNINGS",
								Content = new EventFilesView()
								{
									DataContext = cgbEvent
								}
							};
							window.ShowDialog();
						})));
					else if (hasWarnings)
						AddToMessagesList(MessageVM.CreateWarning(inst, $"Deployed {nFilesDeployed} with WARNINGS.", new DelegateCommand(_ =>
						{
							Window window = new Window
							{
								Title = "Build-Event Details including WARNINGS",
								Content = new EventFilesView()
								{
									DataContext = cgbEvent
								}
							};
							window.ShowDialog();
						})));
					else
						AddToMessagesList(MessageVM.CreateError(inst, $"Deployed {nFilesDeployed} with ERRORS.", new DelegateCommand(_ =>
						{
							Window window = new Window
							{
								Title = "Build-Event Details including ERRORS",
								Content = new EventFilesView()
								{
									DataContext = cgbEvent
								}
							};
							window.ShowDialog();
						})));
					mustShowMessages = true;
				}
				else
				{
					AddToMessagesList(MessageVM.CreateInfo(inst, $"Deployed {nFilesDeployed} with ERRORS.", new DelegateCommand(_ =>
					{
						Window window = new Window
						{
							Title = "Build-Event Details",
							Content = new EventFilesView()
							{
								DataContext = cgbEvent
							}
						};
						window.ShowDialog();
					})));
				}

				if (mustShowMessages)
				{
					ShowMessagesList();
				}
			}
		}

		public void AddToMessagesList(MessageVM mvm)
		{
			_messagesListVM.Items.Add(mvm);
			ShowMessagesList();
		}

		public void AddToMessagesList(IEnumerable<MessageVM> mvms)
		{
			foreach (var mvm in mvms)
			{
				_messagesListVM.Items.Add(mvm);
			}
			ShowMessagesList();
		}

		public void ShowMessagesList()
		{
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
