using CgbPostBuildHelper.Utils;
using CgbPostBuildHelper.View;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace CgbPostBuildHelper.ViewModel
{
	class WatchedDirectoryVM : BindableBase
	{
		public WatchedDirectoryVM(WpfApplication app, CgbAppInstanceVM inst, string directoryToWatch)
		{
			_app = app;
			_inst = inst;
			_dispatcher = System.Windows.Threading.Dispatcher.CurrentDispatcher;
			Directory = directoryToWatch;
			FileSystemWatcher = new FileSystemWatcher
			{
				Path = Directory,
				// Which kinds of changes to watch for (strangely enough, without Security, no change-events, WTF?!)
				NotifyFilter = NotifyFilters.LastAccess | NotifyFilters.LastWrite | NotifyFilters.FileName | NotifyFilters.Attributes | NotifyFilters.CreationTime | NotifyFilters.Security | NotifyFilters.Size,
				// Files to be watched will be set later
				Filter = "TBD"
			};
			Files = new ObservableCollection<WatchedFileVM>();
		}

		private readonly System.Windows.Threading.Dispatcher _dispatcher;
		private readonly WpfApplication _app;
		private readonly CgbAppInstanceVM _inst; 
		public FileSystemWatcher FileSystemWatcher { get; }
		public string Directory { get; }
		public ObservableCollection<WatchedFileVM> Files { get; }

		public void NightGathersAndNowMyWatchBegins()
		{
			FileSystemWatcher.Filter = "*.*";
			//foreach (var f in Files)
			//{
			//	if (FileSystemWatcher.Filter.Length > 0)
			//		FileSystemWatcher.Filter += ",";
			//	FileSystemWatcher.Filter += f.FileName;
			//}
			Console.WriteLine($"Starting directory watcher on directory '{FileSystemWatcher.Path}' with the following filter: '{FileSystemWatcher.Filter}'");

			FileSystemWatcher.Changed += FileSystemWatcher_Changed;
			FileSystemWatcher.Deleted += FileSystemWatcher_Deleted;
			FileSystemWatcher.Renamed += FileSystemWatcher_Renamed;
			FileSystemWatcher.Created += FileSystemWatcher_Created;
			FileSystemWatcher.EnableRaisingEvents = true;
		}

		private void HandleFileEvent(string filePath)
		{
			if (filePath.Contains("~"))
			{
				return;
			}

			try
			{
				var watchFileEntry = Files.GetFile(filePath);
				if (null == watchFileEntry)
				{
					_dispatcher.Invoke(() =>
					{
						_app.AddToMessagesList(MessageVM.CreateError(_inst, $"Received a file system event for '{filePath}' but couldn't we aren't watching that file, actually.", null)); // TODO: perform some action?
					});
					return;
				}

				_inst.PrepareDeployment(
					_inst.Files,
					filePath, watchFileEntry.FilterPath,
					out var deployment);

				// It can be null, if it is not an asset/shader that should be deployed
				if (null == deployment)
				{
					return;
				}

				// Do it!
				deployment.Deploy();

				bool eventHasErrors = false;
				bool eventHasWarnings = false;
				var nFilesDeployed = 0;

				// -> Store the whole event (but a little bit later)
				var cgbEvent = new CgbEventVM(CgbEventType.Update);

				foreach (var deployedFile in deployment.FilesDeployed)
				{
					var deploymentHasErrors = deployedFile.Messages.ContainsMessagesOfType(MessageType.Error);
					var deploymentHasWarnings = deployedFile.Messages.ContainsMessagesOfType(MessageType.Warning);
					eventHasErrors = eventHasErrors || deploymentHasErrors;
					eventHasWarnings = eventHasWarnings || deploymentHasWarnings;

					// Show errors/warnings in window immediately IF this behavior has been opted-in via our settings
					if (deploymentHasWarnings || deploymentHasErrors)
					{
						if ((CgbPostBuildHelper.Properties.Settings.Default.ShowWindowForVkShaderDeployment && deployment is Deployers.VkShaderDeployment)
							|| (CgbPostBuildHelper.Properties.Settings.Default.ShowWindowForGlShaderDeployment && deployment is Deployers.GlShaderDeployment)
							|| (CgbPostBuildHelper.Properties.Settings.Default.ShowWindowForModelDeployment && deployment is Deployers.ModelDeployment))
						{
							Window window = new Window
							{
								Width = 480,
								Height = 320,
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

				// Now, store the event (...AT THE FRONT)
				_inst.AllEventsEver.Insert(0, cgbEvent);

				// Analyze the outcome, create a message and possibly show it, if there could be a problem (files with warnings or errors)
				if (eventHasErrors || eventHasWarnings)
				{
					void addErrorWarningsList(string message, string title)
					{
						_dispatcher.Invoke(() =>
						{
							_app.AddToMessagesList(MessageVM.CreateError(_inst, message, new DelegateCommand(_ =>
							{
								Window window = new Window
								{
									Width = 960,
									Height = 600,
									Title = title,
									Content = new EventFilesView()
									{
										DataContext = new
										{
											Path = _inst.Path,
											ShortPath = _inst.ShortPath,
											AllEventsEver = new[] { cgbEvent }
										}
									}
								};
								window.Show();
							})));
						});
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
				}
				else
				{
					_dispatcher.Invoke(() =>
					{
							_app.AddToMessagesList(MessageVM.CreateInfo(_inst, $"Deployed {nFilesDeployed} files.", new DelegateCommand(_ =>
						{
							Window window = new Window
							{
								Width = 960,
								Height = 600,
								Title = "Build-Event Details",
								Content = new EventFilesView()
								{
									DataContext = new
									{
										Path = _inst.Path,
										ShortPath = _inst.ShortPath,
										AllEventsEver = new[] { cgbEvent }
									}
								}
							};
							window.Show();
						})));
					});
				}
			}
			catch (Exception ex)
			{
				_dispatcher.Invoke(() =>
				{
					_app.AddToMessagesList(MessageVM.CreateError(_inst, ex.Message, null)); // TODO: perform some action
				});
				return;
			}

		}

		private void FileSystemWatcher_Created(object sender, FileSystemEventArgs e)
		{
			HandleFileEvent(e.FullPath);
		}

		private void FileSystemWatcher_Renamed(object sender, RenamedEventArgs e)
		{
			HandleFileEvent(e.FullPath);
		}

		private void FileSystemWatcher_Deleted(object sender, FileSystemEventArgs e)
		{
			// Do... nothing?
		}

		private void FileSystemWatcher_Changed(object sender, FileSystemEventArgs e)
		{
			HandleFileEvent(e.FullPath);
		}

		public void EndWatchAndDie()
		{
			FileSystemWatcher.EnableRaisingEvents = false;
			FileSystemWatcher.Created -= FileSystemWatcher_Created;
			FileSystemWatcher.Changed -= FileSystemWatcher_Changed;
			FileSystemWatcher.Deleted -= FileSystemWatcher_Deleted;
			FileSystemWatcher.Renamed -= FileSystemWatcher_Renamed;
			Files.Clear();
			FileSystemWatcher.Dispose();
		}
	}
}
