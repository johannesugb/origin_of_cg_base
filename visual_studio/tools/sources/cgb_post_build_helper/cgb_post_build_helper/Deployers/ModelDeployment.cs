using CgbPostBuildHelper.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Text.RegularExpressions;
using Diag = System.Diagnostics;
using System.Security.Cryptography;
using CgbPostBuildHelper.ViewModel;
using Assimp;
using System.Runtime.InteropServices;
using CgbPostBuildHelper.Utils;

namespace CgbPostBuildHelper.Deployers
{
	class ModelDeployment : DeploymentBase
	{
		struct TextureCleanupData
		{
			public string FullOutputPathNormalized { get; set; }
			public string FullInputPathNormalized { get; set; }
			public string OriginalRelativePath { get; set; } // TODO: USE THIS STRUCT instead of Tuple<string, string>, OMG!
		}

		// Use a List, because order matters (maybe)
		List<Tuple<string, string>> _outputToInputPathsNormalized;

		private string GetInputPathForTexture(string textureRelativePath)
		{
			return Path.Combine(_inputFile.DirectoryName, textureRelativePath);
		}

		private string GetOutputPathForTexture(string textureRelativePath)
		{
			return Path.Combine(new FileInfo(_outputFilePath).DirectoryName, textureRelativePath);
		}

		public void SetTextures(IEnumerable<string> textures)
		{
			_texturePaths.AddRange(textures);

			_outputToInputPathsNormalized = new List<Tuple<string, string>>();
			foreach (var txp in _texturePaths)
			{
				var outPath = GetOutputPathForTexture(txp);
				var outPathNrm = CgbUtils.NormalizePath(outPath);
				var existing = (from x in _outputToInputPathsNormalized where x.Item1 == outPathNrm select x).FirstOrDefault();
				if (null == existing)
				{
					var inpPath = GetInputPathForTexture(txp);
					var inpPathNrm = CgbUtils.NormalizePath(inpPath);
					_outputToInputPathsNormalized.Add(Tuple.Create(outPathNrm, inpPathNrm));
				}
			}
		}

		public override bool HasConflictWith(DeploymentBase other)
		{
			if (other is ModelDeployment otherModel)
			{
				System.Diagnostics.Debug.Assert(null != this._outputToInputPathsNormalized); // If that assert fails, SetTextures has not been invoked before
				System.Diagnostics.Debug.Assert(null != otherModel._outputToInputPathsNormalized); // If that assert fails, SetTextures has not been invoked before
				foreach (var pathPair in _outputToInputPathsNormalized)
				{
					var existingInOther = (from x in otherModel._outputToInputPathsNormalized where x.Item1 == pathPair.Item1 select x).FirstOrDefault();
					if (null != existingInOther)
					{
						if (existingInOther.Item2 != pathPair.Item2) // found a conflict! Input paths do not match!
							return true;
					}
				}
			}
			// In any case, perform the check of the base class:
			return base.HasConflictWith(other);
		}

		public override void Deploy()
		{
			var modelOutPath = new FileInfo(_outputFilePath);
			Directory.CreateDirectory(modelOutPath.DirectoryName);

			var assetFileModel = PrepareNewAssetFile(null);
			assetFileModel.FileType = FileType.Generic3dModel;
			assetFileModel.OutputFilePath = modelOutPath.FullName;
			CopyFile(assetFileModel);
			FilesDeployed.Add(assetFileModel);

			foreach (var tp in _texturePaths)
			{
				var actualTexPath = GetOutputPathForTexture(tp);
				var texOutPath = new FileInfo(actualTexPath);
				Directory.CreateDirectory(texOutPath.DirectoryName);

				var assetFileTex = PrepareNewAssetFile(assetFileModel);
				// Alter input path:
				assetFileTex.InputFilePath = GetInputPathForTexture(tp);
				assetFileTex.FileType = FileType.Generic;
				assetFileTex.OutputFilePath = texOutPath.FullName;
				CopyFile(assetFileModel);
				FilesDeployed.Add(assetFileTex);
			}

			assetFileModel.Messages.Add(Message.Create(MessageType.Success, $"Copied model '{assetFileModel.OutputFilePath}', and {_texturePaths.Count} dependent material textures.", null)); // TODO: open a window or so?
		}

		protected readonly List<string> _texturePaths = new List<string>();
	}

}
