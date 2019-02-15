using cgb_overseer.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Text.RegularExpressions;

namespace cgb_overseer.Utils
{
	static class CgbUtils
	{
		static readonly Regex RegexIsInAssets = new Regex(@"assets([\\\/].+)*$", RegexOptions.Compiled | RegexOptions.IgnoreCase);
		static readonly Regex RegexIsInShaders = new Regex(@"shaders([\\\/].+)*$", RegexOptions.Compiled | RegexOptions.IgnoreCase);

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

		public static void AddOrReplaceAssetFile(this List<AssetFile> list, string filePath, string filterPath)
		{
			// There are two special filter paths: "assets" and "shaders".
			// Files under both filter paths are copied to the output directory, but
			// only those under "shaders" are possibly compiled to SPIR-V (in case
			// of building against Vulkan) or their GLSL code might be modified (in
			// case of building against OpenGL)

			var fileInfo = new FileInfo(filePath);
			// TODO: Proceed here with handling one specific file (Analyze filter, file type, etc.)
		}
	}
}
