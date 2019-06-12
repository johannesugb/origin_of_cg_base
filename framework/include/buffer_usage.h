#pragma once

namespace cgb
{
	/** Specifies how a buffer will/might be used */
	enum struct memory_usage
	{
		/** No special usages */
		nothing_special					= 0x0000,

		/** Used for indirect draw calls */
		indirect						= 0x0001,

		/** Used for transform feedback */
		transform_feedback				= 0x0002,

		/** Used for the transform feedback counter  */
		transform_feedback_counter		= 0x0004,

		/** Used for conditional rendering */
		conditional_rendering			= 0x0008,

		/** Used for ray tracing */
		ray_tracing						= 0x0010,

		/** Used for determining the address of a buffer on the device */
		device_address					= 0x0020,
	};

	inline memory_usage operator| (memory_usage a, memory_usage b)
	{
		typedef std::underlying_type<memory_usage>::type EnumType;
		return static_cast<memory_usage>(static_cast<EnumType>(a) | static_cast<EnumType>(b));
	}

	inline memory_usage operator& (memory_usage a, memory_usage b)
	{
		typedef std::underlying_type<memory_usage>::type EnumType;
		return static_cast<memory_usage>(static_cast<EnumType>(a) & static_cast<EnumType>(b));
	}

	inline memory_usage& operator |= (memory_usage& a, memory_usage b)
	{
		return a = a | b;
	}

	inline memory_usage& operator &= (memory_usage& a, memory_usage b)
	{
		return a = a & b;
	}
}
