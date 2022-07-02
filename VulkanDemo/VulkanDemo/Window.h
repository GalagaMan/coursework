#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#define VK_KHR_SURFACE



class Window
{
public:
	GLFWwindow* window_;
	Window(uint32_t width, uint32_t height, const char* title);
	~Window();
	bool ShouldClose();
	void PollEvents();

	uint32_t Width();
	uint32_t Height();
	const char* Title();
private:
	uint32_t width;
	uint32_t height;
	const char* title;

	void CheckGlfwInit();
	void CreateWindow(uint32_t width, uint32_t height, const char* title);
};
