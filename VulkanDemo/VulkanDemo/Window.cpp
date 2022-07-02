#include "Window.h"


void Window::CheckGlfwInit()
{
	if (!glfwInit())
		throw std::runtime_error("glfw could not be initialized");
}

void Window::CreateWindow(uint32_t width, uint32_t height, const char* title)
{
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if(!window_)
		glfwTerminate();
}

Window::Window(uint32_t width, uint32_t height, const char* title)
	:window_(), width(width), height(height), title(title)
{
	CheckGlfwInit();
	CreateWindow(width, height, title);
}

Window::~Window()
{
	glfwDestroyWindow(this->window_);
	glfwTerminate();
}

bool Window::ShouldClose()
{
	return glfwWindowShouldClose(window_);
}

void Window::PollEvents()
{
	glfwPollEvents();
}

uint32_t Window::Width()
{
	return width;
}

uint32_t Window::Height()
{
	return height;
}

const char* Window::Title()
{
	return title;
}
