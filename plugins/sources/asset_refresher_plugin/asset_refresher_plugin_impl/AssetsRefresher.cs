using FileHelpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AssetsRefresherPluginImpl
{
	sealed class CleanupHelper
	{
		public CleanupHelper()
		{
			MessageBox.Show("CleanupHelper created");
		}

		~CleanupHelper()
		{
			MessageBox.Show("Inside CleanupHelper's destructor");
			AssetsRefresher.Cleanup();
			MessageBox.Show("CleanupHelper's destructor finished");
		}
	}

    public static class AssetsRefresher
    {
		// only watch those files which were actually copied into the target folder
		static readonly string[] ActionsToWatch = new [] { "COPIED", "REPLACED" };

		static readonly Dictionary<string, HashSet<string>> FilesInDirectory = new Dictionary<string, HashSet<string>>();
		static readonly Dictionary<string, string> SourcesToDestinations = new Dictionary<string, string>();
		static readonly List<FileSystemWatcher> ActiveFileSystemWatchers = new List<FileSystemWatcher>();
		static readonly CleanupHelper CleanupHelper = new CleanupHelper();

		// Set up files to watch
		public static void RefreshAssets(string pathToAssetsLog)
		{
			DialogResult result = MessageBox.Show($"Path to assets-log file: {pathToAssetsLog}", "C#", MessageBoxButtons.OK);
			
			try
			{
				var engine = new FileHelperEngine<CopyFileLogEntry>();
				var entries = engine.ReadFile(pathToAssetsLog);

				foreach (var entry in entries)
				{
					if (ActionsToWatch.Contains(entry.Action.ToUpper()))
					{
						AddEntryInfo(entry);
					}
				}
			}
			catch (Exception ex)
			{
				MessageBox.Show($"Problem while reading the asset-log file: {ex.Message}");
			}

			// Listen for changes per directry:
			foreach (var directory in FilesInDirectory.Keys)
			{
				// Pass full directory name and the filenames within
				StartWatchingDirectory(directory, (from x in FilesInDirectory[directory] select Path.GetFileName(x)).ToArray());
			}
		}

		public static void Cleanup()
		{
			foreach (var watcher in ActiveFileSystemWatchers)
			{
				watcher.EnableRaisingEvents = false;
				watcher.Changed -= Watcher_Changed;
			}
			ActiveFileSystemWatchers.Clear();
			FilesInDirectory.Clear();
			SourcesToDestinations.Clear();
		}

		private static void AddEntryInfo(CopyFileLogEntry entry)
		{
			var dirName = Path.GetDirectoryName(entry.Source);
			var fileName = entry.Source;
			if (!FilesInDirectory.ContainsKey(dirName))
			{
				FilesInDirectory[dirName] = new HashSet<string>();
			}
			FilesInDirectory[dirName].Add(fileName);
		}

		private static void StartWatchingDirectory(string directory, string[] filenames)
		{
			var watcher = new FileSystemWatcher
			{
				Path = directory,
				// Which kinds of changes to watch for
				NotifyFilter = NotifyFilters.LastAccess | NotifyFilters.LastWrite | NotifyFilters.Attributes | NotifyFilters.CreationTime | NotifyFilters.Security | NotifyFilters.Size,
				// Only watch a single file
				Filter = string.Join(",", filenames)
			};
			watcher.Changed += Watcher_Changed;
			watcher.EnableRaisingEvents = true;
			ActiveFileSystemWatchers.Add(watcher);
		}

		private static void Watcher_Changed(object sender, FileSystemEventArgs e)
		{
			if (SourcesToDestinations.ContainsKey(e.FullPath))
			{
				try
				{
					File.Copy(e.FullPath, SourcesToDestinations[e.FullPath], true);
				}
				catch (Exception ex)
				{
					MessageBox.Show($"Exception during File.Copy (TODO: Remove this MessageBox): {ex.Message}");
				}
			}
			else
			{
				MessageBox.Show("That shouldn't happen: Got a changed-event from a file which is not present in the 'sources to destinations'-dictionary.");
			}
		}
	}
}
