#include "context.h"

namespace cgb
{
	namespace settings
	{
		// Define the global variables:

		std::string gApplicationName = "cg_base Application";

		uint32_t gApplicationVersion = cgb::make_version(1u, 0u, 0u);

		std::vector<const char*> gRequiredInstanceExtensions;

		std::vector<const char*> gValidationLayersToBeActivated = {
			"VK_LAYER_LUNARG_standard_validation"
		};

		std::vector<const char*> gRequiredDeviceExtensions;
	}
}
