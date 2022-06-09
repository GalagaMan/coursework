#include "VkRenderer.h"


VkRenderer::VkRenderer(GLFWwindow* windowHandle)
	:window(windowHandle)
{
	CreateInstance();
	SetUpVkDevice();
	std::cerr << "video adapter utilized: " << m_PhysDevice.getProperties().deviceName << " " << to_string(m_PhysDevice.getProperties().deviceType) << "\n";
	auto const Extensions = m_PhysDevice.enumerateDeviceExtensionProperties();
	for(vk::ExtensionProperties const& extension : Extensions)
	{
		for(auto const charc : extension.extensionName)
		{
			std::cerr << charc;
		}
		std::cerr << "\n";
	}
	InitCommandBuffer();
	SetUpSurface();
	InitSwapchain();
}

VkRenderer::~VkRenderer()
{
	for (auto const& imageView : m_imageViews)
	{
		m_device.destroyImageView(imageView);
	}
	m_device.destroySwapchainKHR();
	m_instance.destroySurfaceKHR(m_surface);
	m_device.freeCommandBuffers(m_command_pool, m_command_buffer);
	m_device.destroyCommandPool(m_command_pool);
	m_device.destroy();
	m_instance.destroy();
}

void VkRenderer::CreateInstance()
{
	ExtensionNames = glfwGetRequiredInstanceExtensions(&ExtensionCount);

	vk::ApplicationInfo application_info
	{
		title,
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

	m_instance = vk::createInstance(instance_info);
	std::cerr << "instance created successfully\n";
}

void VkRenderer::SetUpVkDevice()
{
	m_PhysDevice = m_instance.enumeratePhysicalDevices().front();
	m_queue_family_properties = m_PhysDevice.getQueueFamilyProperties();

	auto const PropertyIterator = std::find_if(m_queue_family_properties.begin(),
		m_queue_family_properties.end(), [](vk::QueueFamilyProperties const& queue_family_properties)
		{return queue_family_properties.queueFlags & vk::QueueFlagBits::eGraphics; });
	
	graphicsQueueFamilyIndex = std::distance(m_queue_family_properties.begin(), PropertyIterator);
	assert(graphicsQueueFamilyIndex < m_queue_family_properties.size());

	constexpr float_t qPriority = 0.0f;
	vk::DeviceQueueCreateInfo DeviceQueueInfo(vk::DeviceQueueCreateFlags{}, (graphicsQueueFamilyIndex), 1, &qPriority);

	m_device = m_PhysDevice.createDevice(vk::DeviceCreateInfo{ vk::DeviceCreateFlags(), DeviceQueueInfo });

	std::cerr << "device set up successfully\n";
}

void VkRenderer::InitCommandBuffer()
{
	m_command_pool = m_device.createCommandPool(vk::CommandPoolCreateInfo{ vk::CommandPoolCreateFlags{}, static_cast<uint32_t>(graphicsQueueFamilyIndex) });

	m_command_buffer = m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{ m_command_pool, vk::CommandBufferLevel::ePrimary, 1 }).front();

	std::cerr << "command buffer initialized successfully\n";
}

void VkRenderer::SetUpSurface()
{
	VkSurfaceKHR surface;
	VkResult const error = glfwCreateWindowSurface(m_instance, window, nullptr, &surface);
	if (error != VK_SUCCESS)
		throw std::runtime_error("failed to create vulkan rendering surface");
	m_surface = vk::SurfaceKHR{ surface };

	m_available_queue_family_index = m_PhysDevice.getSurfaceSupportKHR(static_cast<uint32_t>(graphicsQueueFamilyIndex), m_surface) ? graphicsQueueFamilyIndex : m_queue_family_properties.size();
	if (m_available_queue_family_index == m_queue_family_properties.size())
	{
		for(size_t i{0}; i < m_queue_family_properties.size(); i++)
		{
			if ((m_queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) && m_PhysDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), m_surface))
			{
				graphicsQueueFamilyIndex = static_cast<uint32_t>(i);
				m_available_queue_family_index = static_cast<uint32_t>(i);
				break;
			}
		}
	}
	if(m_available_queue_family_index == m_queue_family_properties.size())
	{
		for(size_t i{0}; i < m_queue_family_properties.size(); i++)
		{
			if (m_PhysDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), m_surface))
			{
				m_available_queue_family_index = static_cast<uint32_t>(i);
				break;
			}
		}
	}
	if ((graphicsQueueFamilyIndex == m_queue_family_properties.size()) || (m_available_queue_family_index == m_queue_family_properties.size()))
		throw std::runtime_error("no queue suitable for graphics found");

	std::vector<vk::SurfaceFormatKHR> const surfaceFormats = m_PhysDevice.getSurfaceFormatsKHR(m_surface);
	if (surfaceFormats.empty())
		throw std::exception("no vulkan rendering surface formats found");
	m_format = (surfaceFormats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : surfaceFormats[0].format;

	m_surface_capabilities = m_PhysDevice.getSurfaceCapabilitiesKHR(m_surface);

	std::cerr << "vulkan rendering surface set up successfully\n";
}

void VkRenderer::InitSwapchain()
{
	vk::Extent2D SwapExtent;
	if (m_surface_capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
	{
		SwapExtent.width = (width, m_surface_capabilities.minImageExtent.width, m_surface_capabilities.maxImageExtent.width);
		SwapExtent.height = (height, m_surface_capabilities.minImageExtent.height, m_surface_capabilities.maxImageExtent.height);
	}
	else
	{
		SwapExtent = m_surface_capabilities.currentExtent;
	}

	vk::PresentModeKHR const swapCurrentMode = vk::PresentModeKHR::eFifo;

	vk::SurfaceTransformFlagBitsKHR precedingTransformation = (m_surface_capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		? vk::SurfaceTransformFlagBitsKHR::eIdentity
		: m_surface_capabilities.currentTransform;

	vk::CompositeAlphaFlagBitsKHR alphaComposite = (m_surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied : (m_surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied : (m_surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
		? vk::CompositeAlphaFlagBitsKHR::eInherit : vk::CompositeAlphaFlagBitsKHR::eOpaque;

	vk::SwapchainCreateInfoKHR swapchainInfo
	{{},
		m_surface,
		m_surface_capabilities.minImageCount,
		m_format,
		vk::ColorSpaceKHR::eSrgbNonlinear,
		SwapExtent,
		1, vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		{},
		precedingTransformation,
		alphaComposite,
		swapCurrentMode,
		VK_TRUE,
		nullptr,
	};

	uint32_t const queueFamilyIndices[2] = { static_cast<uint32_t>(graphicsQueueFamilyIndex), static_cast<uint32_t>(m_available_queue_family_index) };
	if (graphicsQueueFamilyIndex != m_available_queue_family_index)
	{
		swapchainInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	m_swapchain = m_device.createSwapchainKHR(swapchainInfo, nullptr);

	std::vector<vk::Image> swapchainImages = m_device.getSwapchainImagesKHR(m_swapchain);

	m_imageViews.reserve(swapchainImages.size());

	vk::ImageViewCreateInfo imageViewInfo
	{
		vk::ImageViewCreateFlags{},
		{},
		vk::ImageViewType::e2D,
		m_format,
		{},
		{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
	};

	for(auto const & image : swapchainImages)
	{
		imageViewInfo.image = image;
		m_imageViews.push_back(m_device.createImageView(imageViewInfo));
	}

}


