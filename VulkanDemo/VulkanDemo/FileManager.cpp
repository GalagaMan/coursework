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

void FileManager::BuildGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	auto const vertexShaderCode = ReadFile(vertexShaderPath);
	auto const fragmentShaderCode = ReadFile(fragmentShaderPath);

	std::cerr << "vert shader size: " + vertexShaderCode.size();
	std::cerr << "frag shader size: " + fragmentShaderCode.size();
}

FileManager::FileManager(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	BuildGraphicsPipeline(vertexShaderPath, fragmentShaderPath);
}
