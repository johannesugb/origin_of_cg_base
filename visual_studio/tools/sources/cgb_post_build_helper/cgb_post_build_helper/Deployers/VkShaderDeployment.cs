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

	class VkShaderDeployment : DeploymentBase
	{
		private static readonly string VulkanSdkPath = Environment.GetEnvironmentVariable("VULKAN_SDK");
		private static readonly string GlslangValidatorPath = Path.Combine(VulkanSdkPath, @"Bin\glslangValidator.exe");
		private static readonly string GlslangValidatorParams = " -V -o \"{1}\" \"{0}\"";

		public override void SetInputParameters(InvocationParams config, string filterPath, FileInfo inputFile, string outputFilePath)
		{
			base.SetInputParameters(config, filterPath, inputFile, outputFilePath + ".spv");
		}

		public override void Deploy()
		{
			var outFile = new FileInfo(_outputFilePath);
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
					assetFile.Messages.Add(Message.Create(MessageType.Error, line, null)); // TODO: open a window or so?
					numErrors += 1;
				}
				// check for warning:
				else if (line.TrimStart().StartsWith("warn", StringComparison.InvariantCultureIgnoreCase))
				{
					assetFile.Messages.Add(Message.Create(MessageType.Warning, line, null)); // TODO: open a window or so?
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
					assetFile.Messages.Add(Message.Create(MessageType.Information, $"Compiling shader for Vulkan resulted in {numErrors} errors and {numWarnings} warnings:" + Environment.NewLine + Environment.NewLine + sb.ToString(), null));
				else if (numWarnings > 0)
					assetFile.Messages.Add(Message.Create(MessageType.Information, $"Compiling shader for Vulkan resulted in {numWarnings} warnings:" + Environment.NewLine + Environment.NewLine + sb.ToString(), null));
				else
					assetFile.Messages.Add(Message.Create(MessageType.Information, $"Compiling shader for Vulkan resulted in {numErrors} errors:" + Environment.NewLine + Environment.NewLine + sb.ToString(), null));
			}
			else
			{
				assetFile.Messages.Add(Message.Create(MessageType.Success, $"Compiling shader for Vulkan succeeded:" + Environment.NewLine + Environment.NewLine + sb.ToString(), null));
			}

			FilesDeployed.Add(assetFile);
		}

	}

}
