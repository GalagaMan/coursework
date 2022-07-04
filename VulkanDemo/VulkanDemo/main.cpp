#include <iostream>
#include <stdexcept>
#include <future>
#include "data.h"
#include "VkRenderer.h"
#include "Window.h"


#ifdef NDEBUG
bool debug = false;
#elif _DEBUG
bool debug = true;
#endif



void Run()
{
	Window window(800, 800, "Vulkan Demo");

	if (glfwVulkanSupported())
	{
		VkRenderer vkd{ window };

		while (!window.ShouldClose())
		{
			vkd.Draw();
			window.PollEvents();
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
