#pragma once
#include <string>

namespace Path
{
	inline static std::string GetExtension(const std::string& File)
	{
		size_t index = File.find_last_of('.');
		if (index == std::string::npos || index == 0)
		{
			return "";
		}
		return File.substr(index + 1);
	}
}