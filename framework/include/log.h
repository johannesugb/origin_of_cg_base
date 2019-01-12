#pragma once

namespace cgb
{
	// LOG-LEVELS:
	// 0 ... nothing (except debug messages in DEBUG-mode)
	// 1 ... error messages only
	// 2 ... errors and warnings
	// 3 ... errors, warnings and infos
	// 4 ... everything
	#if !defined(LOG_LEVEL)
	#define LOG_LEVEL 4
	#endif

	enum struct log_type
	{
		error,
		warning,
		info,
		verbose,
		debug,
		debug_verbose
	};

	enum struct log_importance
	{
		normal,
		important
	};
	
	extern void set_console_output_color(cgb::log_type level, cgb::log_importance importance);
	extern void reset_console_output_color();
	extern std::mutex gLogMutex;
	
	#if LOG_LEVEL > 0
	#define LOG_ERROR(msg)		do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::error, cgb::log_importance::normal); \
										fmt::print("{}{}{}\n", "ERR:  ", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_ERROR_EM(msg)		do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::error, cgb::log_importance::important); \
										fmt::print("{}{}{}\n", "ERR:  ", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_ERROR__(msg)		do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::error, cgb::log_importance::normal); \
										fmt::print("{}{}\n", "ERR:  ", msg); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_ERROR_EM__(msg)		do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::error, cgb::log_importance::important); \
										fmt::print("{}{}\n", "ERR:  ", msg); \
										cgb::reset_console_output_color(); \
								} while(false)
	#else
	#define LOG_ERROR(msg)
	#define LOG_ERROR_EM(msg)
	#endif

	#if LOG_LEVEL > 1
	#define LOG_WARNING(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::warning, cgb::log_importance::normal); \
										fmt::print("{}{}{}\n", "WARN: ", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_WARNING_EM(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::warning, cgb::log_importance::important); \
										fmt::print("{}{}{}\n", "WARN: ", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_WARNING__(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::warning, cgb::log_importance::normal); \
										fmt::print("{}{}\n", "WARN: ", msg); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_WARNING_EM__(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::warning, cgb::log_importance::important); \
										fmt::print("{}{}\n", "WARN: "); \
										cgb::reset_console_output_color(); \
								} while(false)
	#else 
	#define LOG_WARNING(msg)
	#define LOG_WARNING_EM(msg)
	#endif

	#if LOG_LEVEL > 2
	#define LOG_INFO(msg)		do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::info, cgb::log_importance::normal); \
										fmt::print("{}{}{}\n", "INFO: ", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_INFO_EM(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::info, cgb::log_importance::important); \
										fmt::print("{}{}{}\n", "INFO: ", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_INFO__(msg)		do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::info, cgb::log_importance::normal); \
										fmt::print("{}{}\n", "INFO: ", msg); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_INFO_EM__(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::info, cgb::log_importance::important); \
										fmt::print("{}{}\n", "INFO: ", msg); \
										cgb::reset_console_output_color(); \
								} while(false)
	#else
	#define LOG_INFO(msg)
	#define LOG_INFO_EM(msg)
	#endif

	#if LOG_LEVEL > 3
	#define LOG_VERBOSE(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::verbose, cgb::log_importance::normal); \
										fmt::print("{}{}{}\n", "VRBS: ", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_VERBOSE_EM(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::verbose, cgb::log_importance::important); \
										fmt::print("{}{}{}\n", "VRBS: ", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_VERBOSE__(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::verbose, cgb::log_importance::normal); \
										fmt::print("{}{}\n", "VRBS: ", msg); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_VERBOSE_EM__(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::verbose, cgb::log_importance::important); \
										fmt::print("{}{}\n", "VRBS: "); \
										cgb::reset_console_output_color(); \
								} while(false)
	#else 
	#define LOG_VERBOSE(msg)
	#define LOG_VERBOSE_EM(msg)
	#endif

	#ifdef _DEBUG
	#define LOG_DEBUG(msg)		do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::debug, cgb::log_importance::normal); \
										fmt::print("{}{}{}\n", "DBG:  ", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_DEBUG_EM(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::debug, cgb::log_importance::important); \
										fmt::print("{}{}{}\n", "DBG:  ", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_DEBUG__(msg)		do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::debug, cgb::log_importance::normal); \
										fmt::print("{}{}\n", "DBG:  ", msg); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_DEBUG_EM__(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::debug, cgb::log_importance::important); \
										fmt::print("{}{}\n", "DBG:  ", msg); \
										cgb::reset_console_output_color(); \
								} while(false)
	#else
	#define LOG_DEBUG(msg)
	#define LOG_DEBUG_EM(msg)
	#endif

	#if defined(_DEBUG) && LOG_LEVEL > 3
	#define LOG_DEBUG_VERBOSE(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::debug_verbose, cgb::log_importance::normal); \
										fmt::print("{}{}{}\n", "DBG-V:", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_DEBUG_VERBOSE_EM(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::debug_verbose, cgb::log_importance::important); \
										fmt::print("{}{}{}\n", "DBG-V:", msg, fmt::format(" | file[{}] line[{}]", cgb::extract_file_name(std::string(__FILE__)), __LINE__)); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_DEBUG_VERBOSE__(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::debug_verbose, cgb::log_importance::normal); \
										fmt::print("{}{}\n", "DBG-V:", msg); \
										cgb::reset_console_output_color(); \
								} while(false)
	#define LOG_DEBUG_VERBOSE_EM__(msg)	do { \
										std::lock_guard<std::mutex> lock(cgb::gLogMutex); \
										cgb::set_console_output_color(cgb::log_type::debug_verbose, cgb::log_importance::important); \
										fmt::print("{}{}\n", "DBG-V:", msg); \
										cgb::reset_console_output_color(); \
								} while(false)
	#else
	#define LOG_DEBUG_VERBOSE(msg)
	#define LOG_DEBUG_VERBOSE_EM(msg)   	
	#endif

	std::string to_string(const glm::mat4&);
	std::string to_string(const glm::mat3&);
	std::string to_string_compact(const glm::mat4&);
	std::string to_string_compact(const glm::mat3&);

	std::string to_string(const glm::vec2&);
	std::string to_string(const glm::vec3&);
	std::string to_string(const glm::vec4&);

	std::string fourcc_to_string(unsigned int fourcc);
}
