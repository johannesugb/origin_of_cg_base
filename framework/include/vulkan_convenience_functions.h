#pragma once

namespace cgb
{
	template <typename V, typename F>
	bool has_flag(V value, F flag)
	{
		return (value & flag) == flag;
	}
}