using cgb_overseer.Utils;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cgb_overseer.ViewModel
{
	class MessagesListViewModel : BindableBase
	{   
		public ObservableCollection<MessageViewModel> Items { get; } = new ObservableCollection<MessageViewModel>();
	}
}
