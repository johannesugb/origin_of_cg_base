#pragma once

namespace cgb
{
	/** \brief Represents the state(s) of a button
	 */
	enum struct key_state
	{
		none        = 0x00,
		pressed     = 0x01,
		down        = 0x02,
		released    = 0x04
	};
}
