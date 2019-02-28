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

namespace CgbPostBuildHelper.Utils
{
	interface IFileDeployment
	{
		void SetInputParameters(CgbAppInstance inst, string filterPath, FileInfo inputFile, string outputFilePath);
		void Deploy();
		List<FileDeploymentData> FilesDeployed { get; }
	}

	abstract class DeploymentBase : IFileDeployment
	{
		public enum SymbolicLink
		{
			File = 0,
			Directory = 1
		}

		[DllImport("kernel32.dll")]
		public static extern bool CreateSymbolicLink(string lpSymlinkFileName, string lpTargetFileName, SymbolicLink dwFlags);

		public List<FileDeploymentData> FilesDeployed => _filesDeployed;

		/// <summary>
		/// Deploys the file by copying it from source to target
		/// </summary>
		protected static void CopyFile(FileDeploymentData deploymentData)
		{
			File.Copy(deploymentData.InputFilePath, deploymentData.OutputFilePath, true);
			deploymentData.DeploymentType = DeploymentType.Copy;
		}

		/// <summary>
		/// Deploys the file by creating a symlink at target, which points to source
		/// </summary>
		protected static void SymlinkFile(FileDeploymentData deploymentData)
		{
			var outputFile = new FileInfo(deploymentData.OutputFilePath);
			if (outputFile.Exists)
			{
				File.Delete(outputFile.FullName);
			}
			CreateSymbolicLink(deploymentData.OutputFilePath, deploymentData.InputFilePath, DeploymentBase.SymbolicLink.File);
			deploymentData.DeploymentType = DeploymentType.Symlink;
		}

		public void SetInputParameters(CgbAppInstance inst, string filterPath, FileInfo inputFile, string outputFilePath)
		{
			_instance = inst;
			_filterPath = filterPath;
			_inputFile = inputFile;
			_outputFilePath = outputFilePath;
		}

		public abstract void Deploy();

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

		protected FileDeploymentData PrepareNewAssetFile(FileDeploymentData parent)
		{
			return new FileDeploymentData
			{
				FilterPath = _filterPath,
				InputFilePath = _inputFile.FullName,
				InputFileHash = null, // TODO: use Property InputFileHash if hash is needed,
				Parent = parent
			};
		}

		protected CgbAppInstance _instance;
		protected string _filterPath;
		protected FileInfo _inputFile;
		protected string _outputFilePath;
		protected byte[] _hash;
		protected readonly List<FileDeploymentData> _filesDeployed = new List<FileDeploymentData>();
	}

	class CopyFileDeployment : DeploymentBase
	{
		public override void Deploy()
		{
			var outPath = new FileInfo(_outputFilePath);
			Directory.CreateDirectory(outPath.DirectoryName);

			var assetFileModel = PrepareNewAssetFile(null);
			assetFileModel.FileType = FileType.Generic;
			assetFileModel.OutputFilePath = outPath.FullName;

			// Now, are we going to copy or are we going to symlink?
			if (_instance.Config.Configuration == BuildConfiguration.Publish)
			{
				CopyFile(assetFileModel);
			}
			else
			{
				SymlinkFile(assetFileModel);
			}
			
			FilesDeployed.Add(assetFileModel);
		}

	}

	class VkShaderDeployment : DeploymentBase
	{
		private static readonly string VulkanSdkPath = Environment.GetEnvironmentVariable("VULKAN_SDK");
		private static readonly string GlslangValidatorPath = Path.Combine(VulkanSdkPath, @"Bin\glslangValidator.exe");
		private static readonly string GlslangValidatorParams = " -V -o \"{1}\" \"{0}\"";

		public override void Deploy()
		{
			var outFile = new FileInfo(_outputFilePath + ".spv");
			Directory.CreateDirectory(outFile.DirectoryName);

			var cmdLineParams = string.Format(GlslangValidatorParams, _inputFile.FullName, outFile.FullName);
			var sbOut = new StringBuilder();
			var sbErr = new StringBuilder();

			// Call the other process:
			using (Diag.Process proc = new Diag.Process()
			{
				StartInfo = new Diag.ProcessStartInfo(GlslangValidatorPath, cmdLineParams)
				{
					UseShellExecute = false,
					RedirectStandardOutput = true,
					RedirectStandardError = true,
					CreateNoWindow = true,
					WindowStyle = Diag.ProcessWindowStyle.Hidden,
				}
			})
			{
				proc.Start();
				while (!proc.StandardOutput.EndOfStream)
				{
					sbOut.Append(proc.StandardOutput.ReadLine());
				}
				while (!proc.StandardError.EndOfStream)
				{
					sbErr.Append(proc.StandardError.ReadLine());
				}
			}

			var assetFile = PrepareNewAssetFile(null);
			assetFile.FileType = FileType.GlslShaderForVk;
			assetFile.OutputFilePath = outFile.FullName;
			assetFile.DeploymentType = DeploymentType.MorphedCopy;

			assetFile.Messages.Add(MessageVM.CreateSuccess(_instance, sbOut.ToString(), null)); // TODO: open a window or so?
			assetFile.Messages.Add(MessageVM.CreateError(_instance, sbErr.ToString(), null)); // TODO: open a window or so?

			FilesDeployed.Add(assetFile);
		}

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
			assetFile.FileType = FileType.GlslShaderForGl;
			assetFile.OutputFilePath = _outputFile.FullName;
			assetFile.DeploymentType = DeploymentType.MorphedCopy;

			assetFile.Messages.Add(MessageVM.CreateSuccess(_instance, $"Copied (Vk->Gl morphed) GLSL file to '{outFile.FullName}'", null)); // TODO: open a window or so?

			FilesDeployed.Add(assetFile);
		}

		protected FileInfo _outputFile = null;
	}

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

				Diag.Debug.Assert(FilesDeployed[0].FileType == FileType.Generic3dModel);
				Diag.Debug.Assert(FilesDeployed[0].Parent == null);
				var assetFileMat = PrepareNewAssetFile(FilesDeployed[0]);
				// Alter input path:
				assetFileMat.InputFilePath = Path.Combine(_inputFile.DirectoryName, matFile);
				assetFileMat.FileType = FileType.ObjMaterials;
				assetFileMat.OutputFilePath = matOutPath.FullName;
				CopyFile(assetFileMat);

				assetFileMat.Messages.Add(MessageVM.CreateSuccess(_instance, $"Added materials file '{assetFileMat.OutputFilePath}', of .obj model '{FilesDeployed[0].OutputFilePath}'", null)); // TODO: open a window or so?

				FilesDeployed.Add(assetFileMat);
			}
		}
	}
}
