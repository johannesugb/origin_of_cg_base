#pragma once

namespace cgb
{
	class composition
	{
	public:
		static composition* current();

		virtual timer& time() = 0;
		virtual cg_object* object_at_index(size_t) = 0;
		virtual cg_object* object_by_name(std::string) = 0;
		virtual void start() = 0;
		virtual void stop() = 0;

	protected:
		static void set_current(composition*);

	private:
		static composition* sCurrentComposition;
	};
}
