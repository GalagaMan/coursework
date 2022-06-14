#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

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
	void CheckGlfwInit();
	void CreateWindow(uint32_t width, uint32_t height, const char* title);
};
