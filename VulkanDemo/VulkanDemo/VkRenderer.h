#pragma once
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include "data.h"
#include "FileManager.h"

#define GLFW_INCLUDE_NONE
#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_FORCE_RADIANS

#define TIMEOUT 100000000


class VkRenderer
{
private:
	glm::mat4x4 model{ glm::mat4x4(1.0f) };
	glm::mat4x4 view{ glm::lookAt(glm::vec3{-5.0f, 3.0f, -10.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, -1.0f, 0.0f}) };
	glm::mat4x4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

	glm::mat4x4 clip = glm::mat4x4{ 1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, -1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 0.5f, 0.0f,
						0.0f, 0.0f, 0.5f, 1.0f };

	glm::mat4x4 mvpc = clip * proj * view * model;


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

	vk::Buffer uniform_buffer;
	vk::DeviceMemory uniform_memory;

	vk::DescriptorSetLayout descriptor_set_layout;

	vk::PipelineLayout pipeline_layout;

	vk::DescriptorPool descriptor_pool;
	vk::DescriptorSet descriptor_set;

	vk::RenderPass render_pass;

	std::array<vk::AttachmentDescription, 2> attachment_descriptions;

	vk::ShaderModule vertex_shader_module;
	vk::ShaderModule fragment_shader_module;

	std::array<vk::ImageView, 2> attachments;
	std::vector<vk::Framebuffer> framebuffers;

	vk::Buffer vertex_buffer;
	vk::DeviceMemory vertex_memory;

	vk::Semaphore image_acquired_sem;

	std::array<vk::ClearValue, 2> clear_values;

	vk::Fence fence;

	uint32_t FindMemoryType(vk::PhysicalDeviceMemoryProperties const& memProperties, uint32_t typeBits, vk::MemoryPropertyFlags requiredBitmask);
	void GetShader(vk::ShaderStageFlagBits const stageBits, std::string& glslShader, std::vector<uint32_t>& spvShader);

	void CreateInstance();
	void SetUpVkDevice();
	void InitCommandBuffer();
	void SetUpSurface();
	void InitSwapchain();
	void SetUpDepthBuffer();
	void SetUpUniformBuffer();
	void CreatePipelineLayout();
	void InitDescriptorSet();
	void InitRenderpass();
	void InitShaders();
	void SetupFrameBuffer();
	void SetupVertexBuffer();

public:
	void WriteCommandBuffer();
	VkRenderer(GLFWwindow* windowHandle);
	~VkRenderer();
};