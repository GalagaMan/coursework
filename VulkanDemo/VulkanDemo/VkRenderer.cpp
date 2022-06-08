#include "VkRenderer.h"


VkRenderer::VkRenderer()
{
	CreateInstance();
	SetUpVkDevice();
	std::cerr << "video adapter utilized: " << m_PhysDevice.getProperties().deviceName << " " << to_string(m_PhysDevice.getProperties().deviceType) << "\n";
	auto const Extensions = m_PhysDevice.enumerateDeviceExtensionProperties();
	for(auto const extension : Extensions)
	{
		for(auto const charc : extension.extensionName)
		{
			std::cerr << charc;
		}
		std::cerr << "\n";
	}
	InitCommandBuffer();
	SetUpSurface();
}

VkRenderer::~VkRenderer()
{
	m_device.freeCommandBuffers(m_command_pool, m_command_buffer);
	m_device.destroyCommandPool(m_command_pool);
	m_device.destroy();
	instance.destroy();
}

void VkRenderer::CreateInstance()
{
	ExtensionNames = glfwGetRequiredInstanceExtensions(&ExtensionCount);

	vk::ApplicationInfo application_info
	{
		"Vulkan 1.3 Demo",
		VK_MAKE_VERSION(1, 0, 0),
		"Vulkan",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_3
	};

	vk::InstanceCreateInfo instance_info
	{
		{},
		&application_info,
		{},{},
		ExtensionCount,
		ExtensionNames
	};

	instance = vk::createInstance(instance_info);
	std::cerr << "instance created successfully\n";
}

void VkRenderer::SetUpVkDevice()
{
	m_PhysDevice = instance.enumeratePhysicalDevices().front();
	m_queue_family_properties = m_PhysDevice.getQueueFamilyProperties();

	auto const PropertyIterator = std::find_if(m_queue_family_properties.begin(),
		m_queue_family_properties.end(), [](vk::QueueFamilyProperties const& queue_family_properties)
		{return queue_family_properties.queueFlags & vk::QueueFlagBits::eGraphics; });
	
	graphicQueueIndex = std::distance(m_queue_family_properties.begin(), PropertyIterator);
	assert(graphicQueueIndex < m_queue_family_properties.size());

	constexpr float_t qPriority = 0.0f;
	vk::DeviceQueueCreateInfo DeviceQueueInfo(vk::DeviceQueueCreateFlags{}, (graphicQueueIndex), 1, &qPriority);

	m_device = m_PhysDevice.createDevice(vk::DeviceCreateInfo{ vk::DeviceCreateFlags(), DeviceQueueInfo });

	std::cerr << "device set up successfully\n";
}

void VkRenderer::InitCommandBuffer()
{
	m_command_pool = m_device.createCommandPool(vk::CommandPoolCreateInfo{ vk::CommandPoolCreateFlags{}, static_cast<uint32_t>(graphicQueueIndex) });

	m_command_buffer = m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{ m_command_pool, vk::CommandBufferLevel::ePrimary, 1 }).front();

	std::cerr << "command buffer initialized successfully\n";
}

void VkRenderer::SetUpSurface()
{
	size_t available_queueFamilyIndex = m_PhysDevice.getSurfaceSupportKHR(graphicQueueIndex, m_surface) ? graphicQueueIndex : m_queue_family_properties.size();
	if (available_queueFamilyIndex == m_queue_family_properties.size())
	{
		for(size_t i{0}; i < m_queue_family_properties.size(); i++)
		{
			if ((m_queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) && m_PhysDevice.getSurfaceSupportKHR(i, m_surface))
			{
				graphicQueueIndex = i;
				available_queueFamilyIndex = i;
				break;
			}
		}
	}
	if(available_queueFamilyIndex == m_queue_family_properties.size())
	{
		for(size_t i{0}; i < m_queue_family_properties.size(); i++)
		{
			if (m_PhysDevice.getSurfaceSupportKHR(i, m_surface))
			{
				available_queueFamilyIndex = i;
				break;
			}
		}
	}
	if ((graphicQueueIndex == m_queue_family_properties.size()) || (available_queueFamilyIndex == m_queue_family_properties.size()))
		throw std::runtime_error("no queue suitable for graphics found");

	std::vector<vk::SurfaceFormatKHR> surfaceFormats = m_PhysDevice.getSurfaceFormatsKHR(m_surface);
	if (surfaceFormats.empty())
		throw std::exception("no vulkan rendering surface formats found");
	vk::Format format = (surfaceFormats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : surfaceFormats[0].format;


}

