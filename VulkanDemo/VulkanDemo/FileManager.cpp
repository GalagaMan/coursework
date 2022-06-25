#include "FileManager.h"

#include <cassert>


std::string FileManager::ReadFile(const std::string& filePath)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		//assert(file.is_open());
		throw std::runtime_error("file could not be opened: " + filePath);

	size_t const fSize = file.tellg();
	std::vector<char> buffer(fSize);

	file.seekg(0);
	file.read(buffer.data(), fSize);

	file.close();

	std::string data;

	for(size_t i{0}; i < buffer.size(); i++)
	{
		data.push_back(*(buffer.data() + i));
	}

	return data;
}


