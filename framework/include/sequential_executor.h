#pragma once

namespace cgb
{
	/**	@brief Handle all @ref cg_element sequentially!
	 *
	 *	An executor compatible with the @ref composition class.
	 *	All @ref cg_element instances which are managed by one @ref composition
	 *	are handled through an executor. The @ref sequential_executor updates,
	 *	renders, etc. all of them one after the other.
	 */
	class sequential_executor
	{
	public:
		sequential_executor(composition_interface* pComposition)
			: mParentComposition(pComposition) 
		{}

		void execute_fixed_updates(const std::vector<cg_element*>& elements)
		{
			for (auto& e : elements)
			{
				e->fixed_update();
			}
		}

		void execute_updates(const std::vector<cg_element*>& elements)
		{
			for (auto& e : elements)
			{
				e->update();
			}
		}

		void execute_renders(const std::vector<cg_element*>& elements)
		{
			for (auto& e : elements)
			{
				e->render();
			}
		}

		void execute_render_gizmos(const std::vector<cg_element*>& elements)
		{
			for (auto& e : elements)
			{
				e->render_gizmos();
			}
		}

		void execute_render_guis(const std::vector<cg_element*>& elements)
		{
			for (auto& e : elements)
			{
				e->render_gui();
			}
		}

	private:
		composition_interface* mParentComposition;
	};
}
