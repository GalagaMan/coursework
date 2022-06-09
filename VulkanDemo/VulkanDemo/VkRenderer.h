#pragma once
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "data.h"


class VkRenderer
{
private:
	GLFWwindow* window;

	const char* const* ExtensionNames;
	uint32_t ExtensionCount;
	vk::Instance m_instance;

	//device related, both physical and virtual
	ptrdiff_t graphicsQueueFamilyIndex;
	vk::PhysicalDevice m_PhysDevice;
	vk::Device m_device;
	std::vector<vk::QueueFamilyProperties> m_queue_family_properties;

	vk::CommandPool m_command_pool;
	vk::CommandBuffer m_command_buffer;

	ptrdiff_t m_available_queue_family_index;
	vk::SurfaceCapabilitiesKHR m_surface_capabilities;
	vk::Format m_format;

	//surface
	vk::SurfaceKHR m_surface;

	std::vector<vk::ImageView> m_imageViews;
	vk::SwapchainKHR m_swapchain;
	

	void CreateInstance();
	void SetUpVkDevice();
	void InitCommandBuffer();
	void SetUpSurface();
	void InitSwapchain();
	//setup surface
	

public:
	VkRenderer(GLFWwindow* windowHandle);
	~VkRenderer();
};