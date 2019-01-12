#include "log.h"

namespace cgb
{
	std::mutex gLogMutex;

	void set_console_output_color(cgb::log_type level, cgb::log_importance importance)
	{
#ifdef _WIN32
		static auto std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		switch (level) {
		case cgb::log_type::error:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0xCF);
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0xC);
				break;
			}
			break;
		case cgb::log_type::warning:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0xE0);
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0xE);
				break;
			}
			break;
		case cgb::log_type::verbose:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0x80);
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0x8);
				break;
			}
			break;
		case cgb::log_type::debug:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0xA0);
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0xA);
				break;
			}
			break;
		case cgb::log_type::debug_verbose:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0x20);
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0x2);
				break;
			}
			break;
		default:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0xF0);
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0xF);
				break;
			}
			break;
		}
#endif // WIN32
	}

	void reset_console_output_color()
	{
#ifdef _WIN32
		static auto std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(std_output_handle, 0xF);
#endif // WIN32
	}


	std::string to_string(const glm::mat4& pMatrix)
	{
		char buf[128];
		sprintf_s(buf, 128,
			"\t%.3f\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\t%.3f\n",
			pMatrix[0][0], pMatrix[0][1], pMatrix[0][2], pMatrix[0][3],
			pMatrix[1][0], pMatrix[1][1], pMatrix[1][2], pMatrix[1][3],
			pMatrix[2][0], pMatrix[2][1], pMatrix[2][2], pMatrix[2][3],
			pMatrix[3][0], pMatrix[3][1], pMatrix[3][2], pMatrix[3][3]);
		return buf;
	}

	std::string to_string(const glm::mat3& pMatrix)
	{
		char buf[128];
		sprintf_s(buf, 128,
			"\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\n",
			pMatrix[0][0], pMatrix[0][1], pMatrix[0][2],
			pMatrix[1][0], pMatrix[1][1], pMatrix[1][2],
			pMatrix[2][0], pMatrix[2][1], pMatrix[2][2]);
		return buf;
	}

	std::string to_string_compact(const glm::mat4& pMatrix)
	{
		char buf[128];
		sprintf_s(buf, 128,
			"{{%.2f, %.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f, %.2f}}\n",
			pMatrix[0][0], pMatrix[0][1], pMatrix[0][2], pMatrix[0][3],
			pMatrix[1][0], pMatrix[1][1], pMatrix[1][2], pMatrix[1][3],
			pMatrix[2][0], pMatrix[2][1], pMatrix[2][2], pMatrix[2][3],
			pMatrix[3][0], pMatrix[3][1], pMatrix[3][2], pMatrix[3][3]);
		return buf;
	}

	std::string to_string_compact(const glm::mat3& pMatrix)
	{
		char buf[128];
		sprintf_s(buf, 128,
			"{{%.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f}}\n",
			pMatrix[0][0], pMatrix[0][1], pMatrix[0][2],
			pMatrix[1][0], pMatrix[1][1], pMatrix[1][2],
			pMatrix[2][0], pMatrix[2][1], pMatrix[2][2]);
		return buf;
	}


	std::string to_string(const glm::vec2& pVector)
	{
		char buf[64];
		sprintf_s(buf, 64, "(%.2f, %.2f)", pVector.x, pVector.y);
		return buf;
	}

	std::string to_string(const glm::vec3& pVector)
	{
		char buf[64];
		sprintf_s(buf, 64, "(%.2f, %.2f, %.2f)", pVector.x, pVector.y, pVector.z);
		return buf;
	}

	std::string to_string(const glm::vec4& pVector)
	{
		char buf[64];
		sprintf_s(buf, 64, "(%.2f, %.2f, %.2f, %.2f)", pVector.x, pVector.y, pVector.z, pVector.w);
		return buf;
	}

	std::string fourccToString(unsigned int fourcc)
	{
		char fourccBuf[8];
		fourccBuf[3] = static_cast<char>(0x000000FF & fourcc);
		fourccBuf[2] = static_cast<char>(0x000000FF & (fourcc >> 8));
		fourccBuf[1] = static_cast<char>(0x000000FF & (fourcc >> 16));
		fourccBuf[0] = static_cast<char>(0x000000FF & (fourcc >> 24));

		// convert 000000000 to spaces
		for (int i = 0; i < 4; i++)
		{
			if (0 == fourccBuf[i])
				fourccBuf[i] = ' ';
		}

		fourccBuf[4] = 0;
		return std::string(fourccBuf);
	}
}
