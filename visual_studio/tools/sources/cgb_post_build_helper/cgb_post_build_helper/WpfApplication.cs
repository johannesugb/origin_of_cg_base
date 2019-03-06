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
using System.Collections.ObjectModel;

namespace CgbPostBuildHelper
{
	/// <summary>
	/// The only instance of our application which SingleApplicationInstance interfaces with
	/// in order to start new actions, instruct file watches, etc.
	/// </summary>
	class WpfApplication : Application, IMessageListLifetimeHandler
	{
		//static readonly Regex RegexFilterEntry = new Regex(@"<(None|Object|Image)\s+.*?Include\s*?\=\s*?\""(.*?)\""\s*?\>.*?\<Filter\s*?.*?\>(.*?)\<\/Filter\>.*?\<\/\1\>", 
		static readonly Regex RegexFilterEntry = new Regex(@"<(None|Object|Image)\s+[^\""]*?Include\s*?\=\s*?\""([^\""]*?)\""\s*?\>\s*?\<Filter\s*?\>([^\<]*?)\<\/Filter\>\s*?\<\/\1\>", 
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
		private readonly ObservableCollection<CgbAppInstanceVM> _instances = new ObservableCollection<CgbAppInstanceVM>();

		public ObservableCollection<CgbAppInstanceVM> AllInstances => _instances;

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
			_taskbarIcon.ContextMenu.DataContext = new ContextMenuActionsVM(this);
			_taskbarIcon.LeftClickCommand = new DelegateCommand(_ => ShowMessagesList());
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
				inst = new CgbAppInstanceVM
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

				var eventHasErrors = false;
				var eventHasWarnings = false;
				var nFilesDeployed = 0;
				var mustShowMessages = false;

				// -> Store the whole event (but a little bit later)
				var cgbEvent = new CgbEventVM(CgbEventType.Build);
				
				// -> Parse the .filters file and deploy each and every file
				int n = filters.Count;
				for (int i=0; i < n; ++i)
				{
					Match match = filters[i];

					string filePath;
					string filterPath;
					try
					{
						filePath = Path.Combine(filtersFile.DirectoryName, match.Groups[2].Value);
						filterPath = match.Groups[3].Value;
					}
					catch (Exception ex)
					{
						Console.WriteLine("Skipping file, because: " + ex.Message);
						continue;
					}

					try
					{
						inst.PrepareDeployment(
							prevAssetsList, 
							filePath, filterPath, 
							out var deployment);

						// It can be null, if it is not an asset/shader that should be deployed
						if (null == deployment)
						{
							continue;
						}

						// Do it!
						deployment.Deploy();

						foreach (var deployedFile in deployment.FilesDeployed)
						{
							// For the current files list:
							inst.Files.Add(deployedFile);

							var deploymentHasErrors = deployedFile.Messages.ContainsMessagesOfType(MessageType.Error);
							var deploymentHasWarnings = deployedFile.Messages.ContainsMessagesOfType(MessageType.Warning);
							eventHasErrors = eventHasErrors || deploymentHasErrors;
							eventHasWarnings = eventHasWarnings || deploymentHasWarnings;

							// Show errors/warnings in window immediately IF this behavior has been opted-in via our settings
							if (deploymentHasWarnings || deploymentHasErrors)
							{
								if (   (CgbPostBuildHelper.Properties.Settings.Default.ShowWindowForVkShaderDeployment && deployment is Deployers.VkShaderDeployment)
									|| (CgbPostBuildHelper.Properties.Settings.Default.ShowWindowForGlShaderDeployment && deployment is Deployers.GlShaderDeployment)
									|| (CgbPostBuildHelper.Properties.Settings.Default.ShowWindowForModelDeployment && deployment is Deployers.ModelDeployment))
								{
									Window window = new Window
									{
										Width = 480, Height = 320,
										Title = "Messages for file " + filePath,
										Content = new MessagesList()
										{
											DataContext = new { Items = deployedFile.Messages }
										}
									};
									window.Show();
								}
							}

							// For the event:
							cgbEvent.Files.Add(deployedFile);
						}
						nFilesDeployed += deployment.FilesDeployed.Count;
					}
					catch (Exception ex)
					{
						AddToMessagesList(MessageVM.CreateError(inst, ex.Message, null)); // TODO: perform some action
						mustShowMessages = true;
						continue;
					}
				}

				// Now, store the event (...AT THE FRONT)
				inst.AllEventsEver.Insert(0, cgbEvent);
					
				// Analyze the outcome, create a message and possibly show it, if there could be a problem (files with warnings or errors)
				if (eventHasErrors || eventHasWarnings)
				{
					void addErrorWarningsList(string message, string title)
					{
						AddToMessagesList(MessageVM.CreateError(inst, message, new DelegateCommand(_ =>
						{
							Window window = new Window
							{
								Width = 960, Height = 600,
								Title = title,
								Content = new EventFilesView()
								{
									DataContext = new 
									{
										Path = inst.Path,
										ShortPath = inst.ShortPath,
										AllEventsEver = new [] { cgbEvent }
									}
								}
							};
							window.Show();
						})));
					}

					if (eventHasWarnings && eventHasErrors)
					{
						addErrorWarningsList($"Deployed {nFilesDeployed} files with ERRORS and WARNINGS.", "Build-Event Details including ERRORS and WARNINGS");
					}
					else if (eventHasWarnings)
					{
						addErrorWarningsList($"Deployed {nFilesDeployed} files with WARNINGS.", "Build-Event Details including WARNINGS");
					}
					else
					{
						addErrorWarningsList($"Deployed {nFilesDeployed} files with ERRORS.", "Build-Event Details including ERRORS");
					}
					mustShowMessages = true;
				}
				else
				{
					AddToMessagesList(MessageVM.CreateInfo(inst, $"Deployed {nFilesDeployed} files.", new DelegateCommand(_ =>
					{
						Window window = new Window
						{
							Width = 960, Height = 600,
							Title = "Build-Event Details",
							Content = new EventFilesView()
							{
								DataContext = new
								{
									Path = inst.Path,
									ShortPath = inst.ShortPath,
									AllEventsEver = new[] { cgbEvent }
								}
							}
						};
						window.Show();
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
			RemoveOldMessagesFromList();
			ShowMessagesList();
		}

		public void AddToMessagesList(IEnumerable<MessageVM> mvms)
		{
			foreach (var mvm in mvms)
			{
				_messagesListVM.Items.Add(mvm);
			}
			RemoveOldMessagesFromList();
			ShowMessagesList();
		}

		public void RemoveOldMessagesFromList()
		{
			var cutoffDate = DateTime.Now - TimeSpan.FromMinutes(10.0);
			for (int i = _messagesListVM.Items.Count - 1; i >= 0; --i)
			{
				if (_messagesListVM.Items[i].CreateDate < cutoffDate)
				{
					_messagesListVM.Items.RemoveAt(i);
				}
			}
		}

		public void ShowMessagesList()
		{
			Console.WriteLine("Show Messages list NOW " + DateTime.Now);
			_taskbarIcon.ShowCustomBalloon(_messagesListView, System.Windows.Controls.Primitives.PopupAnimation.None, null);
			CloseMessagesListLater(true);
		}

		public void ClearMessagesList()
		{
			_messagesListVM.Items.Clear();
			_taskbarIcon.CloseBalloon();
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
