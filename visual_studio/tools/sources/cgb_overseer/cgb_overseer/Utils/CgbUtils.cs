using cgb_overseer.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Text.RegularExpressions;
using Diag = System.Diagnostics;
using System.Security.Cryptography;
using cgb_overseer.ViewModel;
using Assimp;

namespace cgb_overseer.Utils
{
	static class CgbUtils
	{
		private static readonly Regex RegexIsInAssets = new Regex(@"assets([\\\/].+)*$", 
			RegexOptions.Compiled | RegexOptions.IgnoreCase);
		private static readonly Regex RegexIsInShaders = new Regex(@"shaders([\\\/].+)*$", 
			RegexOptions.Compiled | RegexOptions.IgnoreCase);

		private static readonly MD5 Md5Implementation = MD5.Create();
		
		/// <summary>
		/// Calculates the MD5 hash of a file
		/// </summary>
		/// <param name="filePath">Path to the file</param>
		/// <returns>The has as byte-array</returns>
		public static byte[] CalculateFileHash(string filePath)
		{
			byte[] hash;
			using (var inputStream = File.Open(filePath, FileMode.Open, FileAccess.Read, FileShare.Read))
			{
				hash = Md5Implementation.ComputeHash(inputStream);
			}
			return hash;
		}
		
		/// <summary>
		/// Determines if the two given hashes are equal
		/// </summary>
		/// <param name="hash1">First hash to compare with each other</param>
		/// <param name="hash2">Second hash to compare with each other</param>
		/// <returns>true if they are equal</returns>
		static bool AreHashesEqual(byte[] hash1, byte[] hash2)
		{
			return hash1.SequenceEqual(hash2);
		}

		/// <summary>
		/// Helper function for extracting a value for a named argument out of the command line arguments.
		/// Names and values always come in pairs of the form: "-paramName C:\parameter_value"
		/// </summary>
		/// <param name="argName">Name of the argument we are looking for</param>
		/// <param name="args">All command line parameters</param>
		/// <returns>String containing the requested argument's associated value.</returns>
		private static string ExtractValueForNamedArgument(string argName, string[] args)
		{
			argName = argName.Trim();

			if (!argName.StartsWith("-"))
			{
				throw new ArgumentException($"'{argName}' is not a valid argument name; valid argument names start with '-'.");
			}

			for (int i=0; i < args.Length; ++i)
			{
				if (string.Compare(argName, args[i].Trim(), true) == 0)
				{
					if (args.Length == i + 1)
					{
						throw new ArgumentException($"There is no value after the argument named '{argName}'");
					}
					if (args[i+1].Trim().StartsWith("-"))
					{
						throw new ArgumentException($"Invalid value following argument name '{argName}' or value not present after it.");
					}
					// all good (hopefully)
					return args[i+1].Trim();
				}
			}

			throw new ArgumentException($"There is no argument named '{argName}'.");
		}

		/// <summary>
		/// Parse command line arguments and put them into an InvocationParams struct, if parsing was successful.
		/// </summary>
		/// <param name="args">Command line arguments</param>
		/// <returns>A new instance of InvocationParams</returns>
		public static InvocationParams ParseCommandLineArgs(string[] args)
		{
			var configuration = ExtractValueForNamedArgument("-configuration", args).ToLowerInvariant();
			var p = new InvocationParams
			{
				CgbFrameworkPath = ExtractValueForNamedArgument("-framework", args),
				TargetApi = configuration.Contains("opengl") 
							? BuildTargetApi.OpenGL 
							: configuration.Contains("vulkan")
							  ? BuildTargetApi.Vulkan
							  : throw new ArgumentException("Couldn't determine the build target API from the '-configuration' argument's value."),
				Configuration = configuration.Contains("debug")
								? BuildConfiguration.Debug
								: configuration.Contains("release")
								  ? BuildConfiguration.Release
								  : throw new ArgumentException("Couldn't determine the build configuration from the '-configuration' argument's value."),
				Platform = ExtractValueForNamedArgument("-platform", args).ToLower() == "x64" 
						   ? BuildPlatform.x64 
						   : throw new ArgumentException("Target platform does not seem to be 'x64'."),
				VcxprojPath = ExtractValueForNamedArgument("-vcxproj", args),
				FiltersPath = ExtractValueForNamedArgument("-filters", args),
				OutputPath = ExtractValueForNamedArgument("-output", args),
				ExecutablePath = ExtractValueForNamedArgument("-executable", args)
			};
			return p;
		}

		/// <summary>
		/// Finds a specific instance in the list of instances. An instance is uniquely identified by a path
		/// </summary>
		/// <param name="list">Search space</param>
		/// <param name="path">Key</param>
		/// <returns>An instance with matching key or null</returns>
		public static InstanceData GetInstance(this List<InstanceData> list, string path) => (from x in list where string.Compare(x.Path, path, true) == 0 select x).FirstOrDefault();	

		/// <summary>
		/// Adds a new inststance to the collection or updates existing instance data
		/// </summary>
		/// <param name="list">Search space</param>
		/// <param name="data">Data to be added or updated</param>
		/// <returns>The newly added or the updated instance</returns>
		public static InstanceData AddOrUpdateInstance(this List<InstanceData> list, InstanceData data)
		{
			var instance = GetInstance(list, data.Path);
			if (null == instance)
			{
				list.Add(data);
				return list.Last();
			}
			else
			{
				instance.AssignData(data);
				return instance;
			}
		}

		public static void DeployFile(this InstanceData inst, List<AssetFile> oldList, string filePath, string filterPath, out List<AssetFile> outDeployedFiles, out List<MessageViewModel> outErrorSuccessMessages)
		{
			outDeployedFiles = new List<AssetFile>();
			outErrorSuccessMessages = new List<MessageViewModel>();
			try
			{
				if (null == oldList)
				{
					// Make our lifes easier
					oldList = new List<AssetFile>();
				}
				// Prepare for what there is to come:
				var inputFile = new FileInfo(filePath);
				if (!inputFile.Exists)
				{
					outErrorSuccessMessages.Add(MessageViewModel.CreateError($"File '{filePath}' does not exist"));
					return;
				}
				
				// There are two special filter paths: "assets" and "shaders".
				// Files under both filter paths are copied to the output directory, but
				// only those under "shaders" are possibly compiled to SPIR-V (in case
				// of building against Vulkan) or their GLSL code might be modified (in
				// case of building against OpenGL)
				//
				// All in all, we have the following special cases:
				//  #1: If it is a shader file and we're building for Vulkan => compile to SPIR-V
				//  #2: If it is a shader file and we're building for OpenGL => modify GLSL
				//  #3: If it is an .obj 3D Model file => get its materials file

				var isAsset = RegexIsInAssets.IsMatch(filterPath);
				var isShader = RegexIsInShaders.IsMatch(filterPath);
				if (!isAsset && !isShader)
				{
					outErrorSuccessMessages.Add(MessageViewModel.CreateInfo($"Skipping '{filePath}' since it is neither in 'assets/' nor in 'shaders/'"));
					return;
				}

				// Ensure we have no coding errors for the following cases:
				Diag.Debug.Assert(isAsset != isShader);
				Diag.Debug.Assert(null == (from x in inst.Files where string.Compare(x.InputFilePath, filePath, true) == 0 select x).FirstOrDefault());

				// Construct the deployment and DO IT... JUST DO IT
				IFileDeployment deploy = null;
				if (isShader)
				{
					if (inst.Config.TargetApi == BuildTargetApi.Vulkan)
					{
						// It's a shader and we're building for Vulkan => Special case #1
						deploy = new VkShaderDeployment();
					}
					else
					{
						Diag.Debug.Assert(inst.Config.TargetApi == BuildTargetApi.OpenGL);
						// It's a shader and we're building for OpenGL => Special case #2
						deploy = new GlShaderDeployment();
					}
				}
				else // is an asset
				{
					// Is it a model?
					try
					{
						using (AssimpContext importer = new AssimpContext())
						{
							var model = importer.ImportFile(inputFile.FullName);
							var allTextures = new HashSet<string>();
							foreach (var m in model.Materials)
							{
								if (m.HasTextureAmbient)		allTextures.Add(m.TextureAmbient.FilePath);
								if (m.HasTextureDiffuse)		allTextures.Add(m.TextureDiffuse.FilePath);
								if (m.HasTextureDisplacement)	allTextures.Add(m.TextureDisplacement.FilePath);
								if (m.HasTextureEmissive)		allTextures.Add(m.TextureEmissive.FilePath);
								if (m.HasTextureHeight)			allTextures.Add(m.TextureHeight.FilePath);
								if (m.HasTextureLightMap)		allTextures.Add(m.TextureLightMap.FilePath);
								if (m.HasTextureNormal)			allTextures.Add(m.TextureNormal.FilePath);
								if (m.HasTextureOpacity)		allTextures.Add(m.TextureOpacity.FilePath);
								if (m.HasTextureReflection)		allTextures.Add(m.TextureReflection.FilePath);
								if (m.HasTextureSpecular)		allTextures.Add(m.TextureSpecular.FilePath);
							}

							if (inputFile.FullName.Trim().EndsWith(".obj", true, System.Globalization.CultureInfo.InvariantCulture))
							{
								deploy = new ObjModelDeployment();
							}
							else
							{
								deploy = new ModelDeployment();
							}
							((ModelDeployment)deploy).SetTextures(allTextures);
						}
					}
					catch (AssimpException aex)
					{
						Console.WriteLine(aex.Message);
						// Maybe it is no model?!
					}

					if (null == deploy)
					{
						deploy = new CopyFileDeployment();
					}
				}
				deploy.SetInputParameters(
					inst.Config, 
					filterPath, 
					inputFile, 
					Path.Combine(inst.Config.OutputPath, filterPath, inputFile.Name));

				// JUST... DO IT, make your dreams come true!
				deploy.Deploy();
				outDeployedFiles.AddRange(deploy.FilesDeployed);
				outErrorSuccessMessages.AddRange(deploy.StatusMessages);
			}
			catch(Exception ex)
			{
				outErrorSuccessMessages.Add(MessageViewModel.CreateError(ex.ToString()));
				return;
			}
		}
	}
}
