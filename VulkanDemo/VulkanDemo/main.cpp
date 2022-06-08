#include <iostream>
#include <stdexcept>
#include "VkRenderer.h"
#include "Window.h"


#ifdef NDEBUG
bool debug = false;
#elif _DEBUG
bool debug = true;
#endif


constexpr uint32_t WIDTH {640};
constexpr uint32_t HEIGHT {480};
const char* title{ "Vulkan 1.3 Demo" };


void Run(/*const Window& window*/ )
{
	InitWindowData window_data{ WIDTH, HEIGHT, title };	
	Window const window(window_data);
	VkRenderer vkd{};
	if (glfwVulkanSupported())
	{
		VkResult const error = glfwCreateWindowSurface(vkd.instance, window.window_, nullptr, &vkd.surface);
		if (error != VK_SUCCESS)
			throw std::runtime_error("failed to create vulkan rendering surface");
		vkd.m_surface = vk::SurfaceKHR{ vkd.surface };
		while (!glfwWindowShouldClose(window.window_))
		{
			glfwWaitEvents();
		}
	}
}

int main()
{
	try
	{
		Run();
	}
	catch (vk::SystemError& error)
	{
		std::cerr << error.what() << " " << error.code();
		return EXIT_FAILURE;
	}
	catch (std::exception& error)
	{
		std::cerr << error.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "unknown exception";
	}
	return EXIT_SUCCESS;
}
