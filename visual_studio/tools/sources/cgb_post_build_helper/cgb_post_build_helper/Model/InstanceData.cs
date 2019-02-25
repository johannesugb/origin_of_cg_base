using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CgbPostBuildHelper.Model
{
	/// <summary>
	/// Contains all the data for one instance which there is:
	/// - Path
	/// - Current/last built configuration (Vk/Gl, etc.)
	/// - Current/last built list of files
	/// </summary>
	class InstanceData
	{
		public string Path => Config.ExecutablePath;
		public InvocationParams Config { get; set; }
		public List<AssetFile> Files { get; set; }

		public void AssignData(InstanceData other)
		{
			Config = other.Config;
			Files = other.Files;
		}
	}
}
