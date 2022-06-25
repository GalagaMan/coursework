#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <vulkan/vulkan.hpp>
#include "Mesh.h"


class FileManager
{
private:
public:
	static void LoadMesh(Mesh const& mesh, vk::Buffer const& buffer);
	static std::string ReadFile(const std::string& filePath);
};

