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
	InitWindowData window_data{ width, height, title };
	Window const window(window_data);

	if (glfwVulkanSupported())
	{
		Vertex vrt{ glm::vec4{0.4, 0.3, 1., 1}, glm::vec4{0.5, -1., -0.3, 1}, glm::vec4{0.3, 0.5, 1., 1.} };

		std::vector<Vertex> vcVrt;

		vcVrt.emplace_back(vrt);
		//vcVrt.emplace_back(Vertex{ glm::vec4{1, 1, 1, 1}, glm::vec4{1, 1, 1, 1} , glm::vec4{1, 1, 1, 1} });

		Mesh msh{ vcVrt };

		std::cout << sizeof(msh);
		VkRenderer vkd{ window.window_ };
		while (!glfwWindowShouldClose(window.window_))
		{
			vkd.Draw();
			glfwPollEvents();
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
