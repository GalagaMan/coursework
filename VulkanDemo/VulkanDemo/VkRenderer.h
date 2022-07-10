#pragma once
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include "Window.h"
#include "data.h"
#include "FileManager.h"

#define GL_KHR_vulkan_glsl
#define GLFW_INCLUDE_NONE
#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_FORCE_RADIANS

#define TIMEOUT 100000000


typedef std::pair<std::list<vk::Buffer>::iterator, std::list<vk::DeviceMemory>::iterator> vertexBufferMemoryPair;

class VkRenderer
{
private:
	glm::mat4x4 model{ glm::mat4x4{1} };
	glm::mat4x4 view{ glm::lookAt(glm::vec3{-5.0f, 3.0f, -10.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, -1.0f, 0.0f}) };
	glm::mat4x4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	//glm::mat4x4 proj = glm::ortho(-3, 3, -3, 3);

	glm::mat4x4 clip = glm::mat4x4{ 1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, -1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 0.5f, 0.0f,
						0.0f, 0.0f, 0.5f, 1.0f };

	glm::mat4x4 mvpc = clip * proj * view * model;


	Window& window;

	const char* const* ExtensionNames;
	uint32_t ExtensionCount;
	vk::Instance instance;

	ptrdiff_t graphics_queue_family_index;
	vk::PhysicalDevice PhysDevice;

	vk::DeviceQueueCreateInfo FindQueue(vk::PhysicalDevice& device, vk::QueueFlagBits queueBits, std::vector<float_t>&& priorities);

	vk::PhysicalDeviceMemoryProperties device_memory_properties;

	vk::Device logical_device;
	std::vector<vk::QueueFamilyProperties> queue_family_properties;

	vk::CommandPool command_pool;
	vk::CommandBuffer command_buffer;

	ptrdiff_t available_queue_family_index;
	vk::SurfaceCapabilitiesKHR surface_capabilities;
	vk::Format format;

	//surface
	vk::SurfaceKHR surface;

	std::vector<vk::ImageView> imageViews;
	vk::SwapchainKHR swapchain;

	vk::Format depth_format;
	vk::ImageTiling image_tiling;

	vk::Image depth_image;
	vk::DeviceMemory depth_mem;
	vk::ImageView depth_view;

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

	std::list<vk::Buffer> vertex_buffers;
	std::list<vk::DeviceMemory> vertex_memory_segments;

	vk::Semaphore image_acquired_sem;
	vk::Semaphore image_has_finished_rendering_sem;

	std::array<vk::ClearValue, 2> clear_values;

	vk::Fence fence;

	vk::Pipeline pipeline;

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
	void BuildGraphicsPipeline();

public:
	void Draw();
	vertexBufferMemoryPair LoadMesh(Mesh const& mesh);
	void UnloadMesh(vertexBufferMemoryPair const& buffer);
	VkRenderer(Window&);
	~VkRenderer();
};