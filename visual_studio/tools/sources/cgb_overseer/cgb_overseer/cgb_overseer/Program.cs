using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cgb_overseer
{
	class Program
	{
		[STAThread]
		public static void Main(string[] args)
		{
			var wrapper = new SingleApplicationInstance();
			wrapper.Run(args);
		}
	}
}
