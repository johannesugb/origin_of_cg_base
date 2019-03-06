using CgbPostBuildHelper.Utils;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace CgbPostBuildHelper.ViewModel
{
	class FileVM : BindableBase
	{
		public ICommand OpenFileCommand
		{
			get => new DelegateCommand(path =>
			{
				var info = new FileInfo((string)path);
				if (!info.Exists)
				{
					return;
				}

				Process.Start(info.FullName);
			});
		}

		public ICommand OpenFolderCommand
		{
			get => new DelegateCommand(path =>
			{
				var info = new FileInfo((string)path);
				if (!info.Directory.Exists)
				{
					return;
				}

				if (info.Exists)
				{
					string argument = "/select, \"" + info.FullName + "\"";
					Process.Start("explorer.exe", argument);
				}
				else
				{
					string argument = "/select, \"" + info.DirectoryName + "\"";
					Process.Start("explorer.exe", argument);
				}
			});
		}
	}
}
