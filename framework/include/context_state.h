#pragma once

namespace cgb
{
	enum struct context_state
	{
		unknown,
		halfway_initialized,
		fully_initialized,
		composition_ending,
		composition_beginning,
		frame_begun,
		frame_updates_done,
		frame_ended,
		about_to_finalize,
		has_finalized
	};
}
