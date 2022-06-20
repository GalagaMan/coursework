#include <iostream>
#include <stdexcept>
#include "data.h"
#include "VkRenderer.h"
#include "Window.h"


#ifdef NDEBUG
bool debug = false;
#elif _DEBUG
bool debug = true;
#endif


void Run(/*const Window& window*/ )
{
	InitWindowData window_data{ width, height, title };	
	Window const window(window_data);
	if (glfwVulkanSupported())
	{
		VkRenderer vkd{window.window_};
		vkd.WriteCommandBuffer();
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
		std::cerr << "vulkan system error: " << error.what() << " " << error.code();
		return EXIT_FAILURE;
	}
	catch (std::exception& error)
	{
		std::cerr << "standard error: " << error.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "unknown exception";
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
