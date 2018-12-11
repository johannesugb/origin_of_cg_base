#pragma once

namespace cgb
{
	/** Makes a Vulkan-compatible version integer based on the three given numbers */
	static constexpr uint32_t make_version(uint32_t major, uint32_t minor, uint32_t patch)
	{
		return (((major) << 22) | ((minor) << 12) | (patch));
	}

	/** Find Case Insensitive Sub String in a given substring 
	 *  Credits: https://thispointer.com/implementing-a-case-insensitive-stringfind-in-c/
	 */
	static size_t find_case_insensitive(std::string data, std::string toFind, size_t startingPos)
	{
		// Convert complete given String to lower case
		std::transform(data.begin(), data.end(), data.begin(), ::tolower);
		// Convert complete given Sub String to lower case
		std::transform(toFind.begin(), toFind.end(), toFind.begin(), ::tolower);
		// Find sub string in given string
		return data.find(toFind, startingPos);
	}
}
