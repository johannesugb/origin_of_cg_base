using CgbPostBuildHelper.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CgbPostBuildHelper.ViewModel
{
	class SettingsVM : BindableBase
	{
		public bool AlwaysDeployReleaseDlls
		{
			get
			{
				return Properties.Settings.Default.AlwaysDeployReleaseDlls;
			}
			set
			{
				Properties.Settings.Default.AlwaysDeployReleaseDlls = value;
				Properties.Settings.Default.Save();
				IssueOnPropertyChanged();
			}
		}

		public bool CopyAssetsToOutputDirectory
		{
			get
			{
				return Properties.Settings.Default.AlwaysCopyNeverSymlink;
			}
			set
			{
				Properties.Settings.Default.AlwaysCopyNeverSymlink = value;
				Properties.Settings.Default.Save();
				IssueOnPropertyChanged();
			}
		}

		public bool DoNotMonitorFiles
		{
			get
			{
				return Properties.Settings.Default.DoNotMonitorFiles;
			}
			set
			{
				Properties.Settings.Default.DoNotMonitorFiles = value;
				Properties.Settings.Default.Save();
				IssueOnPropertyChanged();
			}
		}

		public bool ShowWindowForVulkanShaders
		{
			get
			{
				return Properties.Settings.Default.ShowWindowForVkShaderDeployment;
			}
			set
			{
				Properties.Settings.Default.ShowWindowForVkShaderDeployment = value;
				Properties.Settings.Default.Save();
				IssueOnPropertyChanged();
			}
		}

		public bool ShowWindowForGlShaders
		{
			get
			{
				return Properties.Settings.Default.ShowWindowForGlShaderDeployment;
			}
			set
			{
				Properties.Settings.Default.ShowWindowForGlShaderDeployment = value;
				Properties.Settings.Default.Save();
				IssueOnPropertyChanged();
			}
		}

		public bool ShowWindowForModels
		{
			get
			{
				return Properties.Settings.Default.ShowWindowForModelDeployment;
			}
			set
			{
				Properties.Settings.Default.ShowWindowForModelDeployment = value;
				Properties.Settings.Default.Save();
				IssueOnPropertyChanged();
			}
		}

	}
}
