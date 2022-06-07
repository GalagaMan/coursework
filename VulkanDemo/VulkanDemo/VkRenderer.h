#pragma once
#include <iostream>
#include <vulkan/vulkan.hpp>
//#include <GLFW/glfw3.h>


class VkRenderer
{
private:
	//vulkan instance related
	vk::ApplicationInfo m_application_info
	{
		"Vulkan 1.3 Demo",
		VK_MAKE_VERSION(1, 0, 0),
		"Vulkan",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_3
	};

	vk::InstanceCreateInfo m_instance_info
	{
		{},
		&m_application_info,
	};


	//device related, both physical and virtual
	ptrdiff_t graphicQueueIndex;
	vk::PhysicalDevice m_PhysDevice;
	vk::Device m_device;
	std::vector<vk::QueueFamilyProperties> m_queue_family_properties;

	vk::CommandPool m_command_pool;
	vk::CommandBuffer m_command_buffer;

	
	

	void CreateInstance();
	void SetUpVkDevice();
	void InitCommandBuffer();
	void SetUpSurface();

public:

	vk::SurfaceKHR m_surface;
	vk::Instance instance;

	VkSurfaceKHR surface;
	VkRenderer();
	~VkRenderer();
};