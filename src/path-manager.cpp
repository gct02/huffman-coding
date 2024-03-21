#include "path-manager.h"

bool hasExtension(const std::string& path)
{
	size_t pos = path.find_last_of('.');

	if (pos != std::string::npos && pos != path.length() - 1) {
		return true;
	}

	return false;
}

std::string getExtension(const std::string& path)
{
	size_t extPos = path.find_last_of(".");

	if (extPos == std::string::npos) {
		return "";
	}

	return path.substr(extPos + 1);
}

std::string stripExtension(const std::string& path)
{
	return { path, 0, path.rfind('.') };
}