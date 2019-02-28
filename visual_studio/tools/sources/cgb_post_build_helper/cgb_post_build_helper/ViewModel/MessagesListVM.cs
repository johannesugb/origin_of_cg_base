using CgbPostBuildHelper.Utils;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CgbPostBuildHelper.ViewModel
{
	class MessagesListVM : BindableBase
	{   
		public ObservableCollection<MessageVM> Items { get; } = new ObservableCollection<MessageVM>();
	}
}
