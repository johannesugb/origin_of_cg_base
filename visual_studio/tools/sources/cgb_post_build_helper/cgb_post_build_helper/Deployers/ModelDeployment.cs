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

namespace CgbPostBuildHelper.Deployers
{
	class ModelDeployment : DeploymentBase
	{
		public void SetTextures(IEnumerable<string> textures)
		{
			_texturePaths.AddRange(textures);
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
				var actualTexPath = Path.Combine(modelOutPath.DirectoryName, tp);
				var texOutPath = new FileInfo(actualTexPath);
				Directory.CreateDirectory(texOutPath.DirectoryName);

				var assetFileTex = PrepareNewAssetFile(assetFileModel);
				// Alter input path:
				assetFileTex.InputFilePath = Path.Combine(_inputFile.DirectoryName, tp);
				assetFileTex.FileType = FileType.Generic;
				assetFileTex.OutputFilePath = texOutPath.FullName;
				CopyFile(assetFileModel);
				FilesDeployed.Add(assetFileTex);
			}

			assetFileModel.Messages.Add(MessageVM.CreateSuccess(_instance, $"Copied model '{assetFileModel.OutputFilePath}', and {_texturePaths.Count} dependent material textures.", null)); // TODO: open a window or so?
		}

		protected readonly List<string> _texturePaths = new List<string>();
	}

}
