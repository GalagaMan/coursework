#include "Window.h"


uint32_t Window::CheckGlfwInit()
{
	if (!glfwInit())
		return -1;
	return 0;
}

uint32_t Window::CreateWindow(uint32_t width, uint32_t height, const char* title)
{
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if(!window_)
	{
		glfwTerminate();
		return -1;
	}
	return 0;
}

Window::Window(InitWindowData& window_data)
	:window_()
{
	CheckGlfwInit();
	CreateWindow(window_data.width, window_data.height, window_data.title);
}

Window::~Window()
{
	glfwDestroyWindow(this->window_);
	glfwTerminate();
}
