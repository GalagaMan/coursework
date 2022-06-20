#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iostream>

class FileManager
{
public:
	FileManager(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	static std::string ReadFile(const std::string& filePath);
private:
	void BuildGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
};

