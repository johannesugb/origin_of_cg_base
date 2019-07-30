#include "cg_base.h"

class my_first_cgb_app : public cgb::cg_element
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static vk::VertexInputBindingDescription binding_description()
		{
			return vk::VertexInputBindingDescription()
				.setBinding(0u)
				.setStride(sizeof(Vertex))
				.setInputRate(vk::VertexInputRate::eVertex);
		}

		static auto attribute_descriptions()
		{
			static std::array attribDescs = {
				vk::VertexInputAttributeDescription()
					.setBinding(0u)
					.setLocation(0u)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(static_cast<uint32_t>(offsetof(Vertex, pos)))
				,
				vk::VertexInputAttributeDescription()
					.setBinding(0u)
					.setLocation(1u)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(static_cast<uint32_t>(offsetof(Vertex, color)))
				,
				vk::VertexInputAttributeDescription()
					.setBinding(0u)
					.setLocation(2u)
					.setFormat(vk::Format::eR32G32Sfloat)
					.setOffset(static_cast<uint32_t>(offsetof(Vertex, texCoord)))
			};
			return attribDescs;
		}
	};

	void initialize() override
	{
		auto swapChainFormat = cgb::context().main_window()->swap_chain_image_format();
		auto xxx = cgb::graphics_pipeline_for(
			"shaders/a_triangle.vert",
			"shaders/a_triangle.frag",
			cgb::viewport_depth_scissors_config::from_window(cgb::context().main_window()),
			cgb::attachment::create_color(swapChainFormat, true)
		);


	}

	void update() override
	{
		if (cgb::input().key_pressed(cgb::key_code::h)) {
			// Log a message:
			LOG_INFO_EM("Hello cg_base!");
		}
		if (cgb::input().key_pressed(cgb::key_code::c)) {
			// Center the cursor:
			auto resolution = cgb::context().main_window()->resolution();
			cgb::context().main_window()->set_cursor_pos({ resolution[0] / 2.0, resolution[1] / 2.0 });
		}
		if (cgb::input().key_pressed(cgb::key_code::escape)) {
			// Stop the current composition:
			cgb::current_composition().stop();
		}
	}

private:
	cgb::graphics_pipeline mPipeline;

};

int main()
{
	try {
		// What's the name of our application
		cgb::settings::gApplicationName = "Hello, World!";

		// Create a window and open it
		auto mainWnd = cgb::context().create_window("Hello World Window");
		mainWnd->set_resolution({ 320, 200 });
		mainWnd->set_presentaton_mode(cgb::presentation_mode::vsync);
		mainWnd->open(); 

		// Create an instance of my_first_cgb_app which, in this case,
		// contains the entire functionality of our application. 
		auto element = my_first_cgb_app();

		// Create a composition of:
		//  - a timer
		//  - an executor
		//  - a behavior
		// ...
		auto hello = cgb::composition<cgb::varying_update_timer, cgb::sequential_executor>({
				&element
			});

		// ... and start that composition!
		hello.start();
	}
	catch (std::runtime_error& re)
	{
		LOG_ERROR_EM(re.what());
	}
}


