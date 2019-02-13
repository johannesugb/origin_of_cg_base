using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cgb_overseer.ViewModel
{
	public interface IMessageListLifetimeHandler
	{
		void KeepAlivePermanently();
		void FadeOutOrBasicallyDoWhatYouWantIDontCareAnymore();
	}
}
