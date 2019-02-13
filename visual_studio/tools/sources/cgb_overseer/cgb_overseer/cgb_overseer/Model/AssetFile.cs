using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cgb_overseer.Model
{
	/// <summary>
	/// In order to identify some file types which need special treatment.
	/// If a file does not need special treatment, it's just set to 'Generic'.
	/// </summary>
	enum AssetType
	{
		/// <summary>
		/// Just a generic asset of any type, except for the following...
		/// </summary>
		Generic,

		/// <summary>
		/// A 3D model file. It will be loaded in order to get its 
		/// linked texture files.
		/// </summary>
		Generic3dModel,

		/// <summary>
		/// A 3D model file of the .obj type. They require special treatment
		/// insofar as they can have materials file that lies outside of the 
		/// original .obj and has to be tracked separately for changes.
		/// </summary>
		Obj3dModel,
		/// <summary>
		/// A materials file, which can only exist under a parent Obj3dModel
		/// type file.
		/// </summary>
		ObjMaterials,

		/// <summary>
		/// A .fscene file, describing an ORCA scene, in the FALCOR-format
		/// </summary>
		OrcaScene,

		/// <summary>
		/// A shader file which is intended to be used with OpenGL
		/// </summary>
		GlslShaderForGl,

		/// <summary>
		/// A shader file which is intended to be used with Vulkan
		/// </summary>
		GlslShaderForVk
	}

	/// <summary>
	/// Data about exactly one specific asset file.
	/// </summary>
	class AssetFile
	{
		/// <summary>
		/// The path to the original file
		/// </summary>
		public string InputFilePath { get; set; } 

		/// <summary>
		/// The file hash of the original file.
		/// This is stored in order to detect file changes.
		/// </summary>
		public byte[] InputFileHash { get; set; }

		/// <summary>
		/// The path where the input file is copied to
		/// </summary>
		public string OutputFilePath { get; set; } 

		/// <summary>
		/// Path of the filter as it is stored in the .vcxproj.filters file
		/// </summary>
		public string FilterPath { get; set; }

		/// <summary>
		/// Which kind of asset is it
		/// </summary>
		public AssetType AssetType { get; set; }

		/// <summary>
		/// (Optional) Reference to a parent AssetFile entry.
		/// This will be set for automatically determined assets, 
		/// like it will happen for the textures of 3D models.
		/// 
		/// Whenever the parent asset changes, also its child assets
		/// have to be updated. This is done via those references.
		/// </summary>
		public AssetFile Parent { get; set; }
	}

}
