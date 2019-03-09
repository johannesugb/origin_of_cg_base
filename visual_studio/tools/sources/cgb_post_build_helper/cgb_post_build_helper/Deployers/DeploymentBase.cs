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
				var outputFile = new FileInfo(deploymentData.OutputFilePath); // TODO: Is there a more efficient way which does not require to always delete it?
				if (outputFile.Exists)
				{
					File.Delete(outputFile.FullName);
				}

				File.Copy(deploymentData.InputFilePath, deploymentData.OutputFilePath, true);
				deploymentData.DeploymentType = DeploymentType.Copy;
			}

			if (Properties.Settings.Default.AlwaysCopyNeverSymlink || _config.Configuration == BuildConfiguration.Publish)
			{
				doCopy();
			}
			else
			{
				var outputFile = new FileInfo(deploymentData.OutputFilePath); // TODO: Is there a more efficient way which does not require to always delete it?
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

		public void SetInputParameters(InvocationParams config, string filterPath, FileInfo inputFile, string outputFilePath)
		{
			_config = config;
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

		protected InvocationParams _config;
		protected string _filterPath;
		protected FileInfo _inputFile;
		protected string _outputFilePath;
		protected byte[] _hash;
		protected readonly List<FileDeploymentData> _filesDeployed = new List<FileDeploymentData>();
	}

}
