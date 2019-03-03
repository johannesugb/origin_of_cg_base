using CgbPostBuildHelper.ViewModel;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CgbPostBuildHelper.Utils;
using System.Windows.Media;
using System.Windows.Input;
using System.Windows;
using CgbPostBuildHelper.View;
using System.Diagnostics;

namespace CgbPostBuildHelper.ViewModel
{
	/// <summary>
	/// In order to identify some file types which need special treatment.
	/// If a file does not need special treatment, it's just set to 'Generic'.
	/// </summary>
	enum FileType
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
	/// Indicates how a file has been deployed from a filesystem-point-of-view
	/// </summary>
	enum DeploymentType
	{
		/// <summary>
		/// The file has been copied to the destination, i.e. an exact copy of
		/// the file's source exists in the destination 
		/// </summary>
		Copy,

		/// <summary>
		/// A new file has been established in the destination, but it does not
		/// (neccessarily) have the same contents as the source file because it
		/// has been morphed/transformed/altered/compiled/changed w.r.t. the original. 
		/// </summary>
		MorphedCopy,

		/// <summary>
		/// A symlink has been established in the destination, which points to
		/// the original source file.
		/// </summary>
		Symlink,
	}

	/// <summary>
	/// Data about exactly one specific asset file.
	/// </summary>
	class FileDeploymentData
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

		public string FilterPathPlusFileName => Path.Combine(FilterPath, new FileInfo(InputFilePath).Name);

		/// <summary>
		/// Which kind of file are we dealing with?
		/// </summary>
		public FileType FileType { get; set; }

		public string FileTypeDescription
		{
			get
			{
				switch (this.FileType)
				{
					case FileType.Generic:
						return "Generic file";
					case FileType.Generic3dModel:
						return "3D model file";
					case FileType.ObjMaterials:
						return "Materials file (.mat) of an .obj 3d model";
					case FileType.OrcaScene:
						return "ORCA Scene file";
					case FileType.GlslShaderForGl:
						return "GLSL shader for OpenGL (potentially modified from Vk-GLSL original)";
					case FileType.GlslShaderForVk:
						return "GLSL shader for Vulkan, compiled to SPIR-V";
				}
				return "?FileType?";
			}
		}

		/// <summary>
		/// HOW has the file been deployed?
		/// </summary>
		public DeploymentType DeploymentType { get; set; }

		public string DeploymentTypeDescription
		{
			get
			{
				switch (this.DeploymentType)
				{
					case DeploymentType.Symlink:
						return "Symlink pointing to original";
					case DeploymentType.MorphedCopy:
						return "New file (compiled or otherwise modified w.r.t. original)";
					case DeploymentType.Copy:
						return "Copy of the original";
				}
				return "?DeploymentType?";
			}
		}

		/// <summary>
		/// (Optional) Reference to a parent AssetFile entry.
		/// This will be set for automatically determined assets, 
		/// like it will happen for the textures of 3D models.
		/// 
		/// Whenever the parent asset changes, also its child assets
		/// have to be updated. This is done via those references.
		/// </summary>
		public FileDeploymentData Parent { get; set; }

		/// <summary>
		/// Messages which occured during file deployment or during file update
		/// </summary>
		public ObservableCollection<MessageVM> Messages { get; } = new ObservableCollection<MessageVM>();

		public string MessagesErrorInfo
		{
			get
			{
				var n = Messages.NumberOfMessagesOfType(MessageType.Error);
				return $"{n} error{(n == 1 ? "" : "s")}";
			}
		}
		public string MessagesInformationInfo
		{
			get
			{
				var n = Messages.NumberOfMessagesOfType(MessageType.Information);
				return $"{n} info{(n == 1 ? "" : "s")}";
			}
		}
		public string MessagesSuccessInfo
		{
			get
			{
				var n = Messages.NumberOfMessagesOfType(MessageType.Success);
				return $"{n} success-message{(n == 1 ? "" : "s")}";
			}
		}
		public string MessagesWarningInfo
		{
			get
			{
				var n = Messages.NumberOfMessagesOfType(MessageType.Warning);
				return $"{n} warning{(n == 1 ? "" : "s")}";
			}
		}

		public Brush MessagesErrorInfoColor => Messages.NumberOfMessagesOfType(MessageType.Error) > 0 ? View.Constants.ErrorBrushDark : Brushes.LightGray;
		public Brush MessagesInformationInfoColor => Messages.NumberOfMessagesOfType(MessageType.Information) > 0 ? View.Constants.InfoBrushDark : Brushes.LightGray;
		public Brush MessagesSuccessInfoColor => Messages.NumberOfMessagesOfType(MessageType.Success) > 0 ? View.Constants.SuccessBrushDark : Brushes.LightGray;
		public Brush MessagesWarningInfoColor => Messages.NumberOfMessagesOfType(MessageType.Warning) > 0 ? View.Constants.WarningBrushDark : Brushes.LightGray;

		public ICommand OpenFileCommand
		{
			get => new DelegateCommand(path =>
			{
				var info = new FileInfo((string)path);
				if (!info.Exists)
				{
					return;
				}

				Process.Start(info.FullName);
			});
		}

		public ICommand OpenFolderCommand
		{
			get => new DelegateCommand(path =>
			{
				var info = new FileInfo((string)path);
				if (!info.Directory.Exists)
				{
					return;
				}

				if (info.Exists)
				{
					string argument = "/select, \"" + info.FullName + "\"";
					Process.Start("explorer.exe", argument);
				}
				else
				{
					string argument = "/select, \"" + info.DirectoryName + "\"";
					Process.Start("explorer.exe", argument);
				}
			});
		}

		public ICommand ShowFileMessagesCommand
		{
			get => new DelegateCommand(_ => 
			{
				Window window = new Window
				{
					Width = 480, Height = 320,
					Title = "Messages for file " + FilterPathPlusFileName,
					Content = new MessagesList()
					{
						DataContext = new { Items = Messages }
					}
				};
				window.Show();
			});
		}
	}

}
