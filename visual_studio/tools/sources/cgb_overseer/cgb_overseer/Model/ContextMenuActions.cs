using cgb_overseer.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace cgb_overseer.Model
{
	class ContextMenuActions : BindableBase
	{
		public ICommand ShowWindowCommand { get; set; } = new DelegateCommand(_ => Console.WriteLine("ShowWindowCommand invoked"));
		public ICommand HideWindowCommand { get; set; } = new DelegateCommand(_ => Console.WriteLine("HideWindowCommand invoked"));
		public ICommand ExitApplicationCommand { get; set; } = new DelegateCommand(_ => Application.Current.Shutdown());
	}
}
