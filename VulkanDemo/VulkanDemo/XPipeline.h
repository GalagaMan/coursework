#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iostream>

class XPipeline
{
public:
	XPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
private:
	static std::vector<char> ReadFile(const std::string& filePath);
	void BuildGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
};

