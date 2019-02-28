using CgbPostBuildHelper.ViewModel;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CgbPostBuildHelper.Model
{
	/// <summary>
	/// Contains all the data for one cgb-app-instance.
	/// </summary>
	class CgbAppInstance
	{
		/// <summary>
		/// The (unique) identifier of an application instance.
		/// This is the path of the executable. 
		/// Since the path must be unique on the file system, the identifier is unique as well.
		/// </summary>
		public string Path => Config.ExecutablePath;
		
		/// <summary>
		/// The parameters which have been passed from VisualStudio's custom build step
		/// </summary>
		public InvocationParams Config { get; set; }

		/// <summary>
		/// List of all files which are managed by this app-instance
		/// </summary>
		public ObservableCollection<FileDeploymentData> Files { get; } = new ObservableCollection<FileDeploymentData>();

		/// <summary>
		/// A list of all build/update events that ever happened for this instance
		/// </summary>
		public ObservableCollection<CgbEventVM> AllEvents { get; } = new ObservableCollection<CgbEventVM>();

		/// <summary>
		/// List of all files which are CURRENTLY being watched/monitored by a filesystem-watcher for changes.
		/// Typically, each FileDetailsVM instance will point to one of the entries inside the Files-collection.
		/// If it doesn't, this should actually mean that there is a bug.
		/// </summary>
		public ObservableCollection<string> CurrentlyWatchedFiles { get; } = new ObservableCollection<string>();
		
		/// <summary>
		/// Assigns this instance all the data of the other instance.
		/// The current values are completely destroyed by this. The instance might even change
		/// its "key" (the Path-property) since also that is overwritten by the data of the other.
		/// </summary>
		/// <param name="other">The instance to copy from</param>
		public void AssignData(CgbAppInstance other)
		{
			Config = other.Config;

			Files.Clear();
			foreach (var f in other.Files)
			{
				Files.Add(f);
			}

			AllEvents.Clear();
			foreach (var e in other.AllEvents)
			{
				AllEvents.Add(e);
			}
		}
	}
}
