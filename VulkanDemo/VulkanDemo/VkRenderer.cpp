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
	//auto const InstanceExtensions = instance.enum
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
	instance = vk::createInstance(m_instance_info);
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

}

