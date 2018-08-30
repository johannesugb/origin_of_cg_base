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

	#if LOG_LEVEL > 0
	#define LOG_ERROR(...)		do { printf("ERROR:   "); printf(__VA_ARGS__); printf(" | file[%s] line[%d]\n", __FILE__, __LINE__); } while(0)
	#else
	#define LOG_ERROR(...)
	#endif

	#if LOG_LEVEL > 1
	#define LOG_WARNING(...)	do { printf("WARNING: "); printf(__VA_ARGS__); printf(" | file[%s] line[%d]\n", __FILE__, __LINE__); } while(0)
	#else 
	#define LOG_WARNING(...)
	#endif

	#if LOG_LEVEL > 2
	#define LOG_INFO(...)		do { printf("INFO:    "); printf(__VA_ARGS__); printf(" | file[%s] line[%d]\n", __FILE__, __LINE__); } while(0)
	#else
	#define LOG_INFO(...)
	#endif

	#if LOG_LEVEL > 3
	#define LOG_VERBOSE(...)	do { printf("VERBOSE: "); printf(__VA_ARGS__); printf(" | file[%s] line[%d]\n", __FILE__, __LINE__); } while(0)
	#else 
	#define LOG_VERBOSE(...)
	#endif

	#ifdef _DEBUG
	#define LOG_DEBUG(...)		do { printf("DEBUG:   "); printf(__VA_ARGS__); printf(" | file[%s] line[%d]\n", __FILE__, __LINE__); } while(0)
	#else
	#define LOG_DEBUG(...)      	
	#endif

	#if defined(_DEBUG) && LOG_LEVEL > 3
	#define LOG_DEBUG_VERBOSE(...)		do { printf("DBGVRBSE:"); printf(__VA_ARGS__); printf(" | file[%s] line[%d]\n", __FILE__, __LINE__); } while(0)
	#else
	#define LOG_DEBUG_VERBOSE(...)      	
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
