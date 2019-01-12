#pragma once

namespace cgb
{
#ifdef USE_BACKSPACES_FOR_PATHS
	const char SEP_TO_USE = '\\';
	const char SEP_NOT_TO_USE = '/';
#else
	const char SEP_TO_USE = '/';
	const char SEP_NOT_TO_USE = '\\';
#endif

	static std::string clean_up_path(const std::string& path)
	{
		auto cleaned_up = path;
		int consecutive_sep_cnt = 0;
		for (int i = 0; i < cleaned_up.size(); ++i) {
			if (cleaned_up[i] == SEP_NOT_TO_USE) {
				cleaned_up[i] = SEP_TO_USE;
			}
			if (cleaned_up[i] == SEP_TO_USE) {
				consecutive_sep_cnt += 1;
			}
			else {
				consecutive_sep_cnt = 0;
			}
			if (consecutive_sep_cnt > 1) {
				cleaned_up = cleaned_up.substr(0, i) + (i < cleaned_up.size() - 1 ? cleaned_up.substr(i + 1) : "");
				consecutive_sep_cnt -= 1;
				--i;
			}
		}
		return cleaned_up;
	}

	static std::string extract_file_name(const std::string& path)
	{
		auto cleaned_path = clean_up_path(path);
		auto last_sep_idx = cleaned_path.find_last_of(SEP_TO_USE);
		if (std::string::npos == last_sep_idx) {
			return cleaned_path;
		}
		return cleaned_path.substr(last_sep_idx + 1);
	}

	static std::string extract_base_path(const std::string& path)
	{
		auto cleaned_path = clean_up_path(path);
		auto last_sep_idx = cleaned_path.find_last_of(SEP_TO_USE);
		if (std::string::npos == last_sep_idx) {
			return cleaned_path;
		}
		return cleaned_path.substr(0, last_sep_idx + 1);
	}

	static std::string combine_paths(const std::string& first, const std::string& second)
	{
		return clean_up_path(first + SEP_TO_USE + second);
	}

	static std::string combine_paths(const char* first, const std::string& second)
	{
		return combine_paths(std::string(first), second);
	}

	static std::string combine_paths(const std::string& first, const char* second)
	{
		return combine_paths(first, std::string(second));
	}

	static std::string combine_paths(const char* first, const char* second)
	{
		return combine_paths(std::string(first), std::string(second));
	}
}