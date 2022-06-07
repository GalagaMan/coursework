#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#define VK_KHR_SURFACE


struct InitWindowData
{
	uint32_t width;
	uint32_t height;
	const char* title;
};

class Window
{
public:
	GLFWwindow* window_;
	Window(InitWindowData& window_data);
	~Window();
private:
	uint32_t CheckGlfwInit();
	uint32_t CreateWindow(uint32_t width, uint32_t height, const char* title);
};
