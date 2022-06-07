#include "XPipeline.h"

#include <cassert>


std::vector<char> XPipeline::ReadFile(const std::string& filePath)
{
	std::ifstream file( filePath, std::ios::ate | std::ios::binary );

	if (!file.is_open())
	//assert(file.is_open());
		throw std::runtime_error("file could not be opened: " + filePath);

	size_t const fSize = static_cast<size_t>((file.tellg()));
	std::vector<char> buffer(fSize);

	file.seekg(0);
	file.read(buffer.data(), fSize);

	file.close();
	return buffer;
}

void XPipeline::BuildGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	auto const vertexShaderCode = ReadFile(vertexShaderPath);
	auto const fragmentShaderCode = ReadFile(fragmentShaderPath);

	std::cerr << "vert shader size: " + vertexShaderCode.size();
	std::cerr << "frag shader size: " + fragmentShaderCode.size();
}

XPipeline::XPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	BuildGraphicsPipeline(vertexShaderPath, fragmentShaderPath);
}
