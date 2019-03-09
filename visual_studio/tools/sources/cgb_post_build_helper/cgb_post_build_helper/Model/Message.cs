using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CgbPostBuildHelper.Model
{
	enum MessageType
	{
		Success,
		Information,
		Warning,
		Error,
	}

	class Message
	{
		public static Message Create(MessageType typeOfMessage, string text, Action additionalInfoCommand)
		{
			return new Message()
			{
				_messageType = typeOfMessage,
				_messageText = text,
				_action = additionalInfoCommand
			};
		}

		private readonly DateTime _createDate = DateTime.Now;
		private string _messageText;
		private MessageType _messageType;
		private Action _action;

		public DateTime CreateDate => _createDate;

		public string MessageText 
		{ 
			get => _messageText;
			set => _messageText = value;
		}

		public MessageType MessageType
		{
			get => _messageType;
			set => _messageType = value;
		}

		public Action Action
		{
			get => _action;
			set => _action = value;
		}
	}
}
