using FileHelpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace post_build_copy_helper
{
	class Program
	{
		/// <summary>
		/// Copies files from one directory to another recursively.
		/// An existing destination-file is replaced if the respective source file is newer and kept if it isn't.
		/// 
		/// Additionally, a logfile is writen, containing the actions performed in the following format:
		///    COPIED;C:\source\path\file.x;D:\destination\path\file.x
		///    REPLACED;C:\source\path\file.x;D:\destination\path\file.x
		///    NOT-REPLACED;C:\source\path\file.x;D:\destination\path\file.x
		///    FAILED;C:\source\path\file.x;D:\destination\path\file.x
		/// 
		/// Parameters:
		///  -s  ... followed by space and the path of the source directory 
		///  -d  ... followed by space and the path of the destination directory
		///  -l  ... followed by space and the name of the logfile
		///  -nl ... create new logfile, i.e. delete the contents of an existing logfile
		///  -al ... append to existing logfile, i.e. do not delete existing logfile contents
		/// </summary>
		/// <param name="args"></param>
		static void Main(string[] args)
		{
			string sourceFolder = null;
			string destinationFolder = null;
			string logfileName = null;
			bool? logfileCreateNew = null;

			for (int i=0; i < args.Length; ++i)
			{
				if (args[i] == "-s")
				{
					if (i + 1 >= args.Length)
					{
						MessageBox.Show("Invalid parameters: No source folder specified after parameter -s");
						Environment.Exit(-1);
					}
					sourceFolder = args[++i];
				}
				else if (args[i] == "-d")
				{
					if (i + 1 >= args.Length)
					{
						MessageBox.Show("Invalid parameters: No destination folder specified after parameter -d");
						Environment.Exit(-1);
					}
					destinationFolder = args[++i];
				}
				else if (args[i] == "-l")
				{
					if (i + 1 >= args.Length)
					{
						MessageBox.Show("Invalid parameters: No logfile name specified after parameter -l");
						Environment.Exit(-1);
					}
					logfileName = args[++i];
				}
				else if (args[i] == "-nl")
				{
					logfileCreateNew = true;
				}
				else if (args[i] == "-al")
				{
					logfileCreateNew = false;
				}
			}

			if (null == sourceFolder)
			{
				MessageBox.Show("Missing parameter: source folder");
				Environment.Exit(-1);
			}
			if (null == destinationFolder)
			{
				MessageBox.Show("Missing parameter: destination folder");
				Environment.Exit(-1);
			}
			if (null != logfileName && null == logfileCreateNew)
			{
				MessageBox.Show("Invalid configuration: If you specify log-file, please also state -nl or -al parameter");
				Environment.Exit(-1);
			}

			try
			{

				if (null == logfileName)
				{
					CopyAllFilesFromSourceToDestination(sourceFolder, destinationFolder, new List<CopyFileLogEntry>());
				}
				else
				{
					var engine = new FileHelperEngine<CopyFileLogEntry>();
					var logContents = new List<CopyFileLogEntry>();
					CopyAllFilesFromSourceToDestination(sourceFolder, destinationFolder, logContents);
				
					var destLogFileName = Path.Combine(destinationFolder, logfileName);
					if (File.Exists(destLogFileName) && !logfileCreateNew.Value)
					{
						// exists and should append
						engine.AppendToFile(destLogFileName, logContents);
					}
					else
					{
						// all other cases: doesn't exist or exists and should not append
						engine.WriteFile(destLogFileName, logContents);

					}
				}

			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.Message);
			}
		}

		static void CopyAllFilesFromSourceToDestination(string sourceFolder, string destinationFolder, List<CopyFileLogEntry> logEntries)
		{
			if (Directory.Exists(sourceFolder))
			{
				Directory.CreateDirectory(destinationFolder);
				
				foreach (string filepath in Directory.GetFiles(sourceFolder, "*.*", SearchOption.AllDirectories))
				{
					var destination = filepath.Replace(sourceFolder, destinationFolder);

					// only overwrite if target is newer
					FileInfo srcFile = new FileInfo(filepath);
					FileInfo destFile = new FileInfo(destination);
					
					try
					{	
						Directory.CreateDirectory(destFile.Directory.FullName);
						if (destFile.Exists)
						{
							if (srcFile.LastWriteTime > destFile.LastWriteTime)
							{
								// source file is newer than destination file
								srcFile.CopyTo(destFile.FullName, true);
								logEntries.Add(new CopyFileLogEntry
								{
									Action = "REPLACED",
									Source = srcFile.FullName,
									Destination = destFile.FullName
								});
							}
							else
							{
								// source file is older than destination file
								logEntries.Add(new CopyFileLogEntry
								{
									Action = "NOT-REPLACED",
									Source = srcFile.FullName,
									Destination = destFile.FullName
								});
							}
						}
						else
						{
							// destination file does not exist
							srcFile.CopyTo(destFile.FullName, false);
							logEntries.Add(new CopyFileLogEntry
							{
								Action = "COPIED",
								Source = srcFile.FullName,
								Destination = destFile.FullName
							});
						}

					}
					catch
					{
						logEntries.Add(new CopyFileLogEntry
						{
							Action = "FAILED",
							Source = srcFile.FullName,
							Destination = destFile.FullName
						});
					}
					
				}
			}
		}
	}
}
