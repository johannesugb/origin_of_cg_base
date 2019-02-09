using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Text.RegularExpressions;
using clone_vs_project.Utils;

namespace clone_vs_project.ViewModel
{
    class MainViewModel : BindableBase
    {
		static readonly Regex RegexNonAlphaNumChars = new Regex(@"[^a-zA-Z0-9]", RegexOptions.Compiled);
		static readonly Regex RegexFindAllItemGroups = new Regex(@"\<ItemGroup.+?\<\/ItemGroup\>", RegexOptions.Singleline | RegexOptions.Compiled);
		static readonly Regex RegexClCompile = new Regex(@"\<ClCompile.+\>", RegexOptions.Compiled);
		static readonly Regex RegexClInclude = new Regex(@"\<ClInclude.+\>", RegexOptions.Compiled);
		static readonly Regex RegexProjectGuid = new Regex(@"\<ProjectGuid\>\{(.+)\}\<\/ProjectGuid\>", RegexOptions.Compiled);
		static readonly Regex RegexProjectReference = new Regex(@"\<ProjectReference.+Include=\""([^\$].*)\"".*\>", RegexOptions.Compiled);
		static readonly Regex RegexRootNamespace = new Regex(@"\<RootNamespace\>(.+)\<\/RootNamespace\>", RegexOptions.Compiled);
		static readonly Regex RegexProjectName = new Regex(@"\<ProjectName\>(.+)\<\/ProjectName\>", RegexOptions.Compiled);
		static readonly Regex RegexImportProject = new Regex(@"\<Import.+Project=\""([^\$].*)\"".*\>", RegexOptions.Compiled);
		static readonly Regex RegexSlnProject = new Regex(@"\s*Project\(.+?\)\s*\=\s*\""(.+?)""\s*\,\s*\""(.+?)\""\s*\,\s*\""\{(.+?)\}\""\s*EndProject", RegexOptions.Singleline | RegexOptions.Compiled);
		static readonly Regex RegexSlnGlobalSectionPost = new Regex(@"GlobalSection\s*\(\s*ProjectConfigurationPlatforms\s*\)\s*\=\s*postSolution(.*?\n|\r|\r\n)\s*EndGlobalSection", RegexOptions.Singleline | RegexOptions.Compiled);
		static readonly Regex RegexSlnProjectConfigEntry = new Regex(@"^\s*\{(.+?)\}.*$", RegexOptions.Multiline | RegexOptions.Compiled); // If RegexOptions.Multiline is not set, ^ and $ will match beginning and the end* of the string, not the line like intended.

		public MainViewModel()
		{
			CloneProject = new DelegateCommand(_ =>
			{
				Messages.Clear();
				Messages.Add(MessageViewModel.CreateInfo("Let's go..."));
				var newProjName = NameOfNewProject.Trim();
				if (string.IsNullOrEmpty(newProjName) || newProjName.Contains(' '))
				{
					Messages.Add(MessageViewModel.CreateError("Please enter a valid new project's name (without spaces!)"));
					return;
				}
				var newProjNameAlphaNum = RegexNonAlphaNumChars.Replace(newProjName, "");

				try
				{ 

					// 1. See if there is a .vcxproj-file in the original folder; if not => exit!
					var origDir = new DirectoryInfo(OriginalVsProjFolder);
					var origProjFile = origDir.GetFiles("*.vcxproj", SearchOption.TopDirectoryOnly)[0];
					  Messages.Add(MessageViewModel.CreateInfo("Found .vcxproj-file: " + origProjFile.FullName));
					var origUserFile = origDir.GetFiles("*.vcxproj.user", SearchOption.TopDirectoryOnly)[0];
					  Messages.Add(MessageViewModel.CreateInfo("Found .user-file: " + origUserFile.FullName));
					
					// 2. Copy AND rename 2 files to the target folder: The .vcxproj file and the .vcxproj.user file
					var targetDir = new DirectoryInfo(TargetCopyProjFolder);
					if (targetDir.GetFiles().Count() > 0 || targetDir.GetDirectories().Count() > 0)
					{
						Messages.Add(MessageViewModel.CreateWarning("Target directory '" + targetDir.FullName + "' is not empty"));
					}
					FileInfo targetProjFile;
					FileInfo targetUserFile;
					{
						var targetProjFilePath = Path.Combine(targetDir.FullName, newProjName + ".vcxproj");
						var targetUserFilePath = Path.Combine(targetDir.FullName, newProjName + ".vcxproj.user");
						File.Copy(origProjFile.FullName, targetProjFilePath);
						File.Copy(origUserFile.FullName, targetUserFilePath);
						targetProjFile = new FileInfo(targetProjFilePath);
						targetUserFile = new FileInfo(targetUserFilePath);
						Messages.Add(MessageViewModel.CreateSuccess($"Created '{newProjName}.vcxproj' file in '{targetDir.FullName}'."));
						Messages.Add(MessageViewModel.CreateSuccess($"Created '{newProjName}.vcxproj.user' file in '{targetDir.FullName}'."));
					}

					// 3. Parse target .vcproj-file and modify it...
					var targetProjFileContents = File.ReadAllText(targetProjFile.FullName);
					var itemGroups = RegexFindAllItemGroups.Matches(targetProjFileContents);
					var sb = new StringBuilder();
					var lastIndex = 0;
					foreach (Match itemGroup in itemGroups)
					{
						var part1 = targetProjFileContents.Substring(lastIndex, itemGroup.Index - lastIndex);
						var part2 = itemGroup.Value;
						lastIndex += part1.Length + part2.Length;
					// 3.1 Remove all the <ClCompile/> entries which are childs of an <ItemGroup/> element
						part2 = RegexClCompile.Replace(part2, string.Empty);
					// 3.2 Remove all the <ClInclude/> entries which are childs of an <ItemGroup/> element
						part2 = RegexClInclude.Replace(part2, string.Empty);
						sb.Append(part1).Append(part2);
					}
					sb.Append(targetProjFileContents.Substring(lastIndex));
					targetProjFileContents = sb.ToString();

					// 3.3 Set a newly generated GUID inside the <ProjectGuid/> element
					var newGuid = Guid.NewGuid();
					var newGuidString = newGuid.ToString();
					var oldGuidMatch = RegexProjectGuid.Match(targetProjFileContents);
					if (!oldGuidMatch.Success)
					{
						Messages.Add(MessageViewModel.CreateError("Couldn't find <ProjectGuid> element"));
						return;
					}
					var oldGuidString = oldGuidMatch.Groups[1].Value;
					targetProjFileContents = targetProjFileContents.Replace(oldGuidString, newGuidString);

					// 3.4 Change all relative paths of the <ProjectReference Include="..."> elements, with '...' being the path to change, IF the path does NOT start with a '$' character.
					{ 
						var projRefs = RegexProjectReference.Matches(targetProjFileContents);
						foreach(Match match in projRefs)
						{
							var relPath = match.Groups[1].Value;
							var realPath = Path.Combine(origDir.FullName, relPath);
							var realFile = new FileInfo(realPath);
							var relativeFromTarget = PathNetCore.GetRelativePath(targetDir.FullName, realFile.FullName);

							var updatedProjRef = match.Value.Replace(relPath, relativeFromTarget);
							targetProjFileContents = targetProjFileContents.Replace(match.Value, updatedProjRef);
						}
					}

					// 3.5 Change the content of the <RootNamespace/> element to the new project's name, leaving only alpha-numeric characters
					{ 
						var rootNamespace = RegexRootNamespace.Match(targetProjFileContents);
						if (rootNamespace.Success)
						{
							var oldNamespace = rootNamespace.Groups[1].Value;
							var updatedRootNamespace = rootNamespace.Value.Replace(oldNamespace, newProjNameAlphaNum);
							targetProjFileContents = targetProjFileContents.Replace(rootNamespace.Value, updatedRootNamespace);
						}
					}

					// 3.6 Change the content of the <ProjectName/> element to the new project's name.
					var projectName = RegexProjectName.Match(targetProjFileContents);
					if (projectName.Success)
					{
						var oldName = projectName.Groups[1].Value;
						var updatedRootNamespace = projectName.Value.Replace(oldName, newProjName);
						targetProjFileContents = targetProjFileContents.Replace(projectName.Value, updatedRootNamespace);
					}

					// 3.7 Change all relative paths of the <Import Project="..." /> elements where the path ('...') does NOT start with a '$' character.
					{
						var importProjs = RegexImportProject.Matches(targetProjFileContents);
						foreach (Match match in importProjs)
						{
							var relPath = match.Groups[1].Value;
							var realPath = Path.Combine(origDir.FullName, relPath);
							var realFile = new FileInfo(realPath);
							var relativeFromTarget = PathNetCore.GetRelativePath(targetDir.FullName, realFile.FullName);

							var updatedImportProj = match.Value.Replace(relPath, relativeFromTarget);
							targetProjFileContents = targetProjFileContents.Replace(match.Value, updatedImportProj);
						}
					}

					// We're done with the .vcxproj-file => save it for good.
					File.WriteAllText(targetProjFile.FullName, targetProjFileContents);
					Messages.Add(MessageViewModel.CreateSuccess($"Successfully altered the '{newProjName}.vcxproj' file."));
					
					// 4. IF we should NOT also modify the .sln file => exit
					if (!DoModifySln)
					{
						Messages.Add(MessageViewModel.CreateWarning("Adding the project to a solution-file was not requested (checkbox unticked) => exiting."));
						return;
					}

					// 5. Parse the .sln file and modify it...
					var slnFile = new FileInfo(PathToSln);
					var slnFileContents = File.ReadAllText(slnFile.FullName);

					// 5.1 Duplicate the lines from "Project" to "EndProject" which contains the original project's GUID,
					//     change the name to the new project's name, change the path to the new project's path, change the GUID to the new project's GUID
					{
						var slnProjectMatches = RegexSlnProject.Matches(slnFileContents);
						foreach(Match match in slnProjectMatches)
						{
							var matchGuid = match.Groups[3].Value;
							if (string.Compare(matchGuid, oldGuidString, true) != 0)
							{
								continue; // not the droids, we're looking for
							}
							
							// Start building the new project's entry into the .sln file
							var newProjSb = new StringBuilder(slnFileContents.Substring(match.Groups[0].Index, match.Groups[1].Index - match.Groups[0].Index));

							// A small helper function for adding the parts between two groups of a regex-match
							void AppendNonMatchedPartBetweenGroups(int groupIndexBefore, int? groupIndexAfter)
							{
								var startIndex = match.Groups[groupIndexBefore].Index + match.Groups[groupIndexBefore].Length;
								if (groupIndexAfter.HasValue)
									newProjSb.Append(slnFileContents.Substring(startIndex, match.Groups[groupIndexAfter.Value].Index - startIndex));
								else
									newProjSb.Append(slnFileContents.Substring(startIndex, match.Groups[0].Index + match.Groups[0].Length - startIndex));
							}

							newProjSb.Append(newProjName);
							AppendNonMatchedPartBetweenGroups(1, 2);
							newProjSb.Append(PathNetCore.GetRelativePath(slnFile.DirectoryName, targetProjFile.FullName));
							AppendNonMatchedPartBetweenGroups(2, 3);
							newProjSb.Append(newGuidString);
							AppendNonMatchedPartBetweenGroups(3, null); // till the end

							// got it => insert it as last element
							var indexToInsert = slnProjectMatches[slnProjectMatches.Count-1].Index + slnProjectMatches[slnProjectMatches.Count - 1].Length;
							slnFileContents = slnFileContents.Insert(indexToInsert, newProjSb.ToString());
						}
					}

					// 5.2 Duplicate all the lines between "GlobalSection(ProjectConfigurationPlatforms)" and "EndGlobalSection" which contain the original project's GUID
					//     Insert them just above the "EndGlobalSection"-line.
					{
						var slnGlobalSection = RegexSlnGlobalSectionPost.Match(slnFileContents);
						if (!slnGlobalSection.Success)
						{
							Messages.Add(MessageViewModel.CreateError("Couldn't find the 'GlobalSection(ProjectConfigurationPlatforms) = postSolution' section in the .sln"));
							return;
						}
						var slnGlobalSectionContent = slnGlobalSection.Groups[1].Value;
						var projectEntries = RegexSlnProjectConfigEntry.Matches(slnGlobalSectionContent);
						var newProjEntriesSb = new StringBuilder();
						foreach (Match match in projectEntries)
						{
							if (string.Compare(match.Groups[1].Value, oldGuidString, true) == 0)
							{
								// These ARE the droids, we're looking for
								newProjEntriesSb.Append(match.Value.Replace(match.Groups[1].Value, newGuidString));
							}
						}

						// Insert the copied entries:
						slnFileContents = slnFileContents.Insert(slnGlobalSection.Groups[1].Index + slnGlobalSection.Groups[1].Length, newProjEntriesSb.ToString());
					}

					// Done and done!
					File.WriteAllText(slnFile.FullName, slnFileContents);
					Messages.Add(MessageViewModel.CreateSuccess($"Successfully altered the '{slnFile.Name}' file."));
				}
				catch (Exception ex)
				{
					Messages.Add(MessageViewModel.CreateError(ex.ToString()));
				}
			});
		}

        public string OriginalVsProjFolder
		{
			get => _originalVsProjFolder;
			set => SetProperty(ref _originalVsProjFolder, value);
		}

        public string TargetCopyProjFolder 
		{ 
			get => _targetCopyProjFolder;
			set => SetProperty(ref _targetCopyProjFolder, value);
		}

		public string NameOfNewProject
		{
			get => _nameOfNewProject;
			set => SetProperty(ref _nameOfNewProject, value);
		}

        public bool DoModifySln
		{
			get => _doModifySln;
			set => SetProperty(ref _doModifySln, value);
		}

		public string PathToSln
		{
			get => _pathToSln;
			set => SetProperty(ref _pathToSln, value);
		}

		public ObservableCollection<MessageViewModel> Messages { get; set; } = new ObservableCollection<MessageViewModel>();

		public DelegateCommand CloneProject { get; set; }

		private string _originalVsProjFolder;
		private string _targetCopyProjFolder;
		private string _nameOfNewProject = "hello_my_new_project";
		private bool _doModifySln;
		private string _pathToSln;
		private string _slnFilterPath = "/";
	}
}
