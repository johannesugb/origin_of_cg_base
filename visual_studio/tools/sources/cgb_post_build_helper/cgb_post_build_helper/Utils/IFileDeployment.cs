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
		[DllImport("kernel32.dll", EntryPoint = "CreateSymbolicLinkW", CharSet = CharSet.Unicode, SetLastError = true)]
		private static extern bool CreateSymbolicLink([In] string lpSymlinkFileName, [In] string lpTargetFileName, [In] int dwFlags);

		[DllImport("kernel32.dll", EntryPoint = "GetFinalPathNameByHandleW", CharSet = CharSet.Unicode, SetLastError = true)]
		private static extern int GetFinalPathNameByHandle([In] IntPtr hFile, [Out] StringBuilder lpszFilePath, [In] int cchFilePath, [In] int dwFlags);

		private const int CREATION_DISPOSITION_OPEN_EXISTING = 3;
		private const int FILE_FLAG_BACKUP_SEMANTICS = 0x02000000;
		private const int SYMBOLIC_LINK_FLAG_FILE = 0x0;
		private const int SYMBOLIC_LINK_FLAG_DIRECTORY = 0x1;
		private const int SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE = 0x2;


		public List<FileDeploymentData> FilesDeployed => _filesDeployed;

		/// <summary>
		/// Deploys the file by copying it from source to target, OR
		/// deploys the file by creating a symlink at target, which points to source
		/// </summary>
		protected void CopyFile(FileDeploymentData deploymentData)
		{
			void doCopy()
			{
				File.Copy(deploymentData.InputFilePath, deploymentData.OutputFilePath, true);
				deploymentData.DeploymentType = DeploymentType.Copy;
			}

			if (_instance.Config.Configuration == BuildConfiguration.Publish)
			{
				doCopy();
			}
			else
			{
				var outputFile = new FileInfo(deploymentData.OutputFilePath);
				if (outputFile.Exists)
				{
					File.Delete(outputFile.FullName);
				}

				CreateSymbolicLink(@"\\?\" + deploymentData.OutputFilePath,
								   @"\\?\" + deploymentData.InputFilePath, SYMBOLIC_LINK_FLAG_FILE | SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE);

				// Sadly, it is not guaranteed that this will work
				if (File.Exists(outputFile.FullName)) // Attention: do not use outputfile.Exists since that is cached!
				{
					// symlink worked
					deploymentData.DeploymentType = DeploymentType.Symlink;
				}
				else
				{
					doCopy();
				}
			}
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
			CopyFile(assetFileModel);
			
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
			var sb = new StringBuilder();

			int numErrors = 0;
			int numWarnings = 0;

			var assetFile = PrepareNewAssetFile(null);
			assetFile.FileType = FileType.GlslShaderForVk;
			assetFile.OutputFilePath = outFile.FullName;
			assetFile.DeploymentType = DeploymentType.MorphedCopy;

			void processLine(string line)
			{
				sb.AppendLine(line);
				// check for error:
				if (line.TrimStart().StartsWith("error", StringComparison.InvariantCultureIgnoreCase))
				{
					assetFile.Messages.Add(MessageVM.CreateError(_instance, line, null)); // TODO: open a window or so?
					numErrors += 1;
				}
				// check for warning:
				else if (line.TrimStart().StartsWith("warn", StringComparison.InvariantCultureIgnoreCase))
				{
					assetFile.Messages.Add(MessageVM.CreateWarning(_instance, line, null)); // TODO: open a window or so?
					numWarnings += 1;
				}
			}

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
					processLine(proc.StandardOutput.ReadLine());
				}
				while (!proc.StandardError.EndOfStream)
				{
					processLine(proc.StandardOutput.ReadLine());
				}
			}

			if (numErrors > 0 || numWarnings > 0) 
			{
				if (numErrors > 0 && numWarnings > 0)
					assetFile.Messages.Add(MessageVM.CreateInfo(_instance, $"Compiling shader for Vulkan resulted in {numErrors} errors and {numWarnings} warnings:" + Environment.NewLine + Environment.NewLine + sb.ToString(), null));
				else if(numWarnings > 0)
					assetFile.Messages.Add(MessageVM.CreateInfo(_instance, $"Compiling shader for Vulkan resulted in {numWarnings} warnings:" + Environment.NewLine + Environment.NewLine + sb.ToString(), null));
				else
					assetFile.Messages.Add(MessageVM.CreateInfo(_instance, $"Compiling shader for Vulkan resulted in {numErrors} errors:" + Environment.NewLine + Environment.NewLine + sb.ToString(), null));
			}
			else
			{
				assetFile.Messages.Add(MessageVM.CreateSuccess(_instance, $"Compiling shader for Vulkan succeeded:" + Environment.NewLine + Environment.NewLine + sb.ToString(), null));
			}

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
