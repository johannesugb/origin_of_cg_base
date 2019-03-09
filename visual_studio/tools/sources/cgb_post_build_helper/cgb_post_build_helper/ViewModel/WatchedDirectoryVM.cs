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

		

		private void FileSystemWatcher_Created(object sender, FileSystemEventArgs e)
		{
			_app.HandleFileEvent(e.FullPath, _inst, Files);
		}

		private void FileSystemWatcher_Renamed(object sender, RenamedEventArgs e)
		{
			_app.HandleFileEvent(e.FullPath, _inst, Files);

		}

		private void FileSystemWatcher_Deleted(object sender, FileSystemEventArgs e)
		{
			// Do... nothing?
		}

		private void FileSystemWatcher_Changed(object sender, FileSystemEventArgs e)
		{
			_app.HandleFileEvent(e.FullPath, _inst, Files);

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
