using CgbPostBuildHelper.Utils;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CgbPostBuildHelper.ViewModel
{
	class MessagesListViewModel : BindableBase
	{   
		public ObservableCollection<MessageViewModel> Items { get; } = new ObservableCollection<MessageViewModel>();
	}
}
