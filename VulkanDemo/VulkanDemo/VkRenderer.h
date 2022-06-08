#pragma once
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>


class VkRenderer
{
private:
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
	const char* const* ExtensionNames;
	uint32_t ExtensionCount;
	vk::SurfaceKHR m_surface;
	vk::Instance instance;

	VkSurfaceKHR surface;
	VkRenderer();
	~VkRenderer();
};