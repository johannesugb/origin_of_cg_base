#include "log.h"

namespace cgb
{
	void set_console_output_color(cgb::log_type level, cgb::log_importance importance)
	{
#ifdef _WIN32
		static auto std_output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		switch (level) {
		case cgb::log_type::error:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0xCF); // white on red
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0xC); // red on black
				break;
			}
			break;
		case cgb::log_type::warning:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0xE0); // black on yellow
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0xE); // yellow on black
				break;
			}
			break;
		case cgb::log_type::verbose:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0x80); // black on gray
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0x8); // gray on black
				break;
			}
			break;
		case cgb::log_type::debug:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0xA0); // black on green
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0xA); // green on black
				break;
			}
			break;
		case cgb::log_type::debug_verbose:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0x20); // black on dark green
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0x2); // dark green on black
				break;
			}
			break;
		case cgb::log_type::system:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0xDF); // white on magenta
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0xD); // magenta on black
				break;
			}
			break;
		default:
			switch (importance) {
			case cgb::log_importance::important:
				SetConsoleTextAttribute(std_output_handle, 0xF0); // black on white
				break;
			default:
				SetConsoleTextAttribute(std_output_handle, 0xF); // white on black
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
		SetConsoleTextAttribute(std_output_handle, 0xF); // white on black
#endif // WIN32
	}

#ifdef LOGGING_ON_SEPARATE_THREAD
	std::mutex gLogMutex;

	void dispatch_log(log_pack pToBeLogged)
	{
		static std::mutex sLogMutex;
		static std::condition_variable sCondVar;
		static std::queue<log_pack> sLogQueue;
		static bool sContinueLogging = true;
		static std::thread sLogThread = std::thread([]() {
			
			cgb::set_console_output_color(cgb::log_type::system, cgb::log_importance::important);
			fmt::print("Logger thread started...");
			cgb::reset_console_output_color();
			fmt::print("\n");

			while (sContinueLogging) {
				// Process all messages
				{
					log_pack front;
					bool empty;
					// 1st lock => See if there is something
					{
						std::scoped_lock<std::mutex> guard(sLogMutex);
						empty = sLogQueue.empty();
						if (!empty)
						{
							front = sLogQueue.front();
							sLogQueue.pop();
						}
					}

					if (!sContinueLogging) continue;

					// If we were able to successfully pop an element => handle it, i.e. print it
					if (!empty)
					{
						// ACTUAL LOGGING:
						cgb::set_console_output_color(front.mLogType, front.mLogImportance);
						fmt::print(front.mMessage);
						cgb::reset_console_output_color();

						// 2nd lock => get correct empty-value
						{
							std::scoped_lock<std::mutex> guard(sLogMutex);
							empty = sLogQueue.empty();
							if (!empty) continue;
						}
					}
				}

				if (!sContinueLogging) continue;

				// No more messages => wait
				{
					// Do not take 100% of the CPU
					std::unique_lock<std::mutex> lock(sLogMutex);
					sCondVar.wait(lock);
				}
			}

			cgb::set_console_output_color(cgb::log_type::system, cgb::log_importance::important);
			fmt::print("Logger thread terminating.");
			cgb::reset_console_output_color();
			fmt::print("\n");
		});
		static struct thread_stopper {
			~thread_stopper() {
				{
					std::unique_lock<std::mutex> lock(sLogMutex);
					sContinueLogging = false;
					sCondVar.notify_all();
				}
				sLogThread.join();
			}
		} sThreadStopper;

		// Enqueue the message and wake the logger thread
		std::scoped_lock<std::mutex> guard(sLogMutex);
		sLogQueue.push(pToBeLogged);
		sCondVar.notify_all();
	}
#else
	void dispatch_log(log_pack pToBeLogged)
	{
		cgb::set_console_output_color(pToBeLogged.mLogType, pToBeLogged.mLogImportance);
		fmt::print(pToBeLogged.mMessage);
		cgb::reset_console_output_color();
	}
#endif

	std::string to_string(const glm::mat4& pMatrix)
	{
		char buf[256];
		sprintf_s(buf, 256,
			"\t%.3f\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\t%.3f\n",
			pMatrix[0][0], pMatrix[0][1], pMatrix[0][2], pMatrix[0][3],
			pMatrix[1][0], pMatrix[1][1], pMatrix[1][2], pMatrix[1][3],
			pMatrix[2][0], pMatrix[2][1], pMatrix[2][2], pMatrix[2][3],
			pMatrix[3][0], pMatrix[3][1], pMatrix[3][2], pMatrix[3][3]);
		return buf;
	}

	std::string to_string(const glm::mat3& pMatrix)
	{
		char buf[256];
		sprintf_s(buf, 256,
			"\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\n\t%.3f\t%.3f\t%.3f\n",
			pMatrix[0][0], pMatrix[0][1], pMatrix[0][2],
			pMatrix[1][0], pMatrix[1][1], pMatrix[1][2],
			pMatrix[2][0], pMatrix[2][1], pMatrix[2][2]);
		return buf;
	}

	std::string to_string_compact(const glm::mat4& pMatrix)
	{
		char buf[256];
		sprintf_s(buf, 256,
			"{{%.2f, %.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f, %.2f}, {%.2f, %.2f, %.2f, %.2f}}\n",
			pMatrix[0][0], pMatrix[0][1], pMatrix[0][2], pMatrix[0][3],
			pMatrix[1][0], pMatrix[1][1], pMatrix[1][2], pMatrix[1][3],
			pMatrix[2][0], pMatrix[2][1], pMatrix[2][2], pMatrix[2][3],
			pMatrix[3][0], pMatrix[3][1], pMatrix[3][2], pMatrix[3][3]);
		return buf;
	}

	std::string to_string_compact(const glm::mat3& pMatrix)
	{
		char buf[256];
		sprintf_s(buf, 256,
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
