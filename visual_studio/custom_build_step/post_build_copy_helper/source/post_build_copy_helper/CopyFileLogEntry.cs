using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using FileHelpers;

namespace post_build_copy_helper
{
	[DelimitedRecord(";")]
	class CopyFileLogEntry
	{
		public string Action { get; set; }
		public string Source { get; set; }
		public string Destination { get; set; }
	}
}
