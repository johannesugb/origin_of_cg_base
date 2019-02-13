using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cgb_overseer.Model
{
	class AssetFile
	{
		public string InputFilePath { get; set; }
		public string OutputFilePath { get; set; }
		public string FilterPath { get; set; }
	}

	class ObjModelFile : AssetFile
	{
		public string PathToMaterialsFile { get; set; }
		public List<string> ReferencedFiles { get; set; }
	}

	class OrcaSceneFile : AssetFile
	{
		public List<string> ReferencedFiles { get; set; }
	}

	class ConvertToSpirvFile : AssetFile
	{
	}
}
