#pragma once
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include "data.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_FORCE_RADIANS


class VkRenderer
{
private:
	GLFWwindow* window;

	const char* const* ExtensionNames;
	uint32_t ExtensionCount;
	vk::Instance m_instance;

	//device related, both physical and virtual
	ptrdiff_t m_graphics_queue_family_index;
	vk::PhysicalDevice m_PhysDevice;

	vk::PhysicalDeviceMemoryProperties m_device_memory_properties;

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

	vk::Format m_depth_format;
	vk::ImageTiling m_image_tiling;

	vk::Image m_depth_image;
	vk::DeviceMemory m_depth_mem;
	vk::ImageView m_depth_view;

	void CreateInstance();
	void SetUpVkDevice();
	void InitCommandBuffer();
	void SetUpSurface();
	void InitSwapchain();
	void SetUpDepthBuffer();
	void SetUpUniformBuffer();

public:
	VkRenderer(GLFWwindow* windowHandle);
	~VkRenderer();
};