using FileHelpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AssetsRefresherPluginImpl
{
	[DelimitedRecord(";")]
	class CopyFileLogEntry
	{
		public string Action { get; set; }
		public string Source { get; set; }
		public string Destination { get; set; }
	}
}
