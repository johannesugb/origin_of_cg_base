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

namespace CgbPostBuildHelper.Utils
{
	interface IFileDeployment
	{
		void SetInputParameters(InvocationParams config, string filterPath, FileInfo inputFile, string outputFilePath);
		void Deploy();
		List<MessageViewModel> StatusMessages { get; }
		List<AssetFile> FilesDeployed { get; }
	}

	abstract class DeploymentBase : IFileDeployment
	{
		public void SetInputParameters(InvocationParams config, string filterPath, FileInfo inputFile, string outputFilePath)
		{
			_config = config;
			_filterPath = filterPath;
			_inputFile = inputFile;
			_outputFilePath = outputFilePath;
		}

		public abstract void Deploy();
		public abstract List<MessageViewModel> StatusMessages { get; }
		public abstract List<AssetFile> FilesDeployed { get; }

		public byte[] InputFileHash
		{
			get
			{
				if (null == _hash)
				{
					_hash = CgbUtils.CalculateFileHash(_inputFile.FullName);
				}
				return _hash;
			}
		}

		protected AssetFile PrepareNewAssetFile(AssetFile parent)
		{
			return new AssetFile
			{
				FilterPath = _filterPath,
				InputFilePath = _inputFile.FullName,
				InputFileHash = null, // TODO: use Property InputFileHash if hash is needed,
				Parent = parent
			};
		}

		protected InvocationParams _config;
		protected string _filterPath;
		protected FileInfo _inputFile;
		protected string _outputFilePath;
		protected byte[] _hash;
	}

	class CopyFileDeployment : DeploymentBase
	{
		public override List<MessageViewModel> StatusMessages => _statusMessages;

		public override List<AssetFile> FilesDeployed => _filesDeployed;

		public override void Deploy()
		{
			var outPath = new FileInfo(_outputFilePath);
			Directory.CreateDirectory(outPath.DirectoryName);

			var assetFileModel = PrepareNewAssetFile(null);
			assetFileModel.AssetType = AssetType.Generic;
			assetFileModel.OutputFilePath = outPath.FullName;
			File.Copy(assetFileModel.InputFilePath, assetFileModel.OutputFilePath, true);
			FilesDeployed.Add(assetFileModel);
		}

		protected readonly List<MessageViewModel> _statusMessages = new List<MessageViewModel>();
		protected readonly List<AssetFile> _filesDeployed = new List<AssetFile>();
	}

	class VkShaderDeployment : DeploymentBase
	{
		private static readonly string VulkanSdkPath = Environment.GetEnvironmentVariable("VULKAN_SDK");
		private static readonly string GlslangValidatorPath = Path.Combine(VulkanSdkPath, @"Bin\glslangValidator.exe");
		private static readonly string GlslangValidatorParams = " -V -o \"{1}\" \"{0}\"";

		public override List<MessageViewModel> StatusMessages => _statusMessages;

		public override List<AssetFile> FilesDeployed => _filesDeployed;

		public override void Deploy()
		{
			var outFile = new FileInfo(_outputFilePath + ".spv");
			Directory.CreateDirectory(outFile.DirectoryName);
			var cmdLineParams = string.Format(GlslangValidatorParams, _inputFile.FullName, outFile.FullName);
			var sb = new StringBuilder();

			// Call the other process:
			using (Diag.Process proc = new Diag.Process()
			{
				StartInfo = new Diag.ProcessStartInfo(GlslangValidatorPath, cmdLineParams)
				{
					UseShellExecute = false,
					RedirectStandardOutput = true
				}
			})
			{
				proc.Start();
				while (!proc.StandardOutput.EndOfStream)
				{
					sb.Append(proc.StandardOutput.ReadLine());
				}
			}

			var assetFile = PrepareNewAssetFile(null);
			assetFile.AssetType = AssetType.GlslShaderForVk;
			assetFile.OutputFilePath = outFile.FullName;
			FilesDeployed.Add(assetFile);

			StatusMessages.Add(MessageViewModel.CreateSuccess(sb.ToString()));
		}

		protected readonly List<MessageViewModel> _statusMessages = new List<MessageViewModel>();
		protected readonly List<AssetFile> _filesDeployed = new List<AssetFile>();
	}

	class GlShaderDeployment : DeploymentBase
	{
		private static readonly Dictionary<string, string> GlslVkToGlReplacements = new Dictionary<string, string>()
		{
			{"gl_VertexIndex", "gl_VertexID"},
			{"gl_InstanceIndex", "gl_InstanceID"},
		};
		private static readonly Regex RegexGlslLayoutSetBinding = new Regex(@"(layout\s*\(.*)(set\s*\=\s*\d+\s*\,\s*)(binding\s*\=\s*\d+)(.*\))",
			RegexOptions.Compiled);
		private static readonly Regex RegexGlslLayoutSetBinding2 = new Regex(@"(layout\s*\(.*)(binding\s*\=\s*\d\s*+)(\,\s*set\s*\=\s*\d+)(.*\))",
			RegexOptions.Compiled);

		public override List<MessageViewModel> StatusMessages => _statusMessages;

		public override List<AssetFile> FilesDeployed => _filesDeployed;

		public static string MorphVkGlslIntoGlGlsl(string vkGlsl)
		{
			string glGlsl = GlslVkToGlReplacements.Aggregate(vkGlsl, (current, pair) => current.Replace(pair.Key, pair.Value));
			glGlsl = RegexGlslLayoutSetBinding.Replace(glGlsl, match => match.Groups[1].Value + match.Groups[3].Value + match.Groups[4].Value);
			glGlsl = RegexGlslLayoutSetBinding2.Replace(glGlsl, match => match.Groups[1].Value + match.Groups[2].Value + match.Groups[4].Value);
			// TODO: There is still a lot of work to do
			// See also: https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_vulkan_glsl.txt
			return glGlsl;
		}

		public override void Deploy()
		{
			var outFile = new FileInfo(_outputFilePath);
			Directory.CreateDirectory(outFile.DirectoryName);
			// Read in -> modify -> write out
			string glslCode = File.ReadAllText(_inputFile.FullName);
			File.WriteAllText(_outputFile.FullName, MorphVkGlslIntoGlGlsl(glslCode));

			var assetFile = PrepareNewAssetFile(null);
			assetFile.AssetType = AssetType.GlslShaderForGl;
			assetFile.OutputFilePath = _outputFile.FullName;
			FilesDeployed.Add(assetFile);

			StatusMessages.Add(MessageViewModel.CreateSuccess($"Copied (Vk->Gl morphed) GLSL file to '{outFile.FullName}'"));
		}

		protected FileInfo _outputFile = null;
		protected readonly List<MessageViewModel> _statusMessages = new List<MessageViewModel>();
		protected readonly List<AssetFile> _filesDeployed = new List<AssetFile>();
	}

	class ModelDeployment : DeploymentBase
	{
		public override List<MessageViewModel> StatusMessages => _statusMessages;

		public override List<AssetFile> FilesDeployed => _filesDeployed;

		public void SetTextures(IEnumerable<string> textures)
		{
			_texturePaths.AddRange(textures);
		}

		public override void Deploy()
		{
			var modelOutPath = new FileInfo(_outputFilePath);
			Directory.CreateDirectory(modelOutPath.DirectoryName);
			
			var assetFileModel = PrepareNewAssetFile(null);
			assetFileModel.AssetType = AssetType.Generic3dModel;
			assetFileModel.OutputFilePath = modelOutPath.FullName;
			File.Copy(assetFileModel.InputFilePath, assetFileModel.OutputFilePath, true);
			FilesDeployed.Add(assetFileModel);

			foreach (var tp in _texturePaths)
			{
				var actualTexPath = Path.Combine(modelOutPath.DirectoryName, tp);
				var texOutPath = new FileInfo(actualTexPath);
				Directory.CreateDirectory(texOutPath.DirectoryName);
				
				var assetFileTex = PrepareNewAssetFile(assetFileModel);
				// Alter input path:
				assetFileTex.InputFilePath = Path.Combine(_inputFile.DirectoryName, tp);
				assetFileTex.AssetType = AssetType.Generic;
				assetFileTex.OutputFilePath = texOutPath.FullName;
				File.Copy(assetFileTex.InputFilePath, assetFileTex.OutputFilePath, true);
				FilesDeployed.Add(assetFileTex);
			}

			StatusMessages.Add(MessageViewModel.CreateSuccess($"Copied model '{assetFileModel.OutputFilePath}', and {_texturePaths.Count} dependent material textures."));
		}

		protected readonly List<string> _texturePaths = new List<string>();
		protected readonly List<MessageViewModel> _statusMessages = new List<MessageViewModel>();
		protected readonly List<AssetFile> _filesDeployed = new List<AssetFile>();
	}

	class ObjModelDeployment : ModelDeployment
	{
		public override void Deploy()
		{
			base.Deploy();
			
			string matFile = null;
			using (var sr = new StreamReader(_inputFile.FullName))
			{
				while (!sr.EndOfStream)
				{
					var line = sr.ReadLine();
					if (line.TrimStart().StartsWith("mtllib"))
					{
						// found a .mat-file!
						matFile = line.TrimStart().Substring("mtllib".Length).Trim();
					}
				}
			}

			if (null != matFile)
			{
				var modelOutPath = new FileInfo(_outputFilePath);
				var actualMatPath = Path.Combine(modelOutPath.DirectoryName, matFile);
				var matOutPath = new FileInfo(actualMatPath);
				Directory.CreateDirectory(matOutPath.DirectoryName);

				Diag.Debug.Assert(FilesDeployed[0].AssetType == AssetType.Generic3dModel);
				Diag.Debug.Assert(FilesDeployed[0].Parent == null);
				var assetFileMat = PrepareNewAssetFile(FilesDeployed[0]);
				// Alter input path:
				assetFileMat.InputFilePath = Path.Combine(_inputFile.DirectoryName, matFile);
				assetFileMat.AssetType = AssetType.ObjMaterials;
				assetFileMat.OutputFilePath = matOutPath.FullName;
				File.Copy(assetFileMat.InputFilePath, assetFileMat.OutputFilePath, true);
				FilesDeployed.Add(assetFileMat);

				StatusMessages.Add(MessageViewModel.CreateSuccess($"Added materials file '{assetFileMat.OutputFilePath}', of .obj model '{FilesDeployed[0].OutputFilePath}'"));
			}
		}
	}
}
