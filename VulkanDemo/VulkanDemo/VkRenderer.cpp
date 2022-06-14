#include "VkRenderer.h"


VkRenderer::VkRenderer(GLFWwindow* windowHandle)
	:window(windowHandle)
{
	CreateInstance();
	SetUpVkDevice();
	std::cerr << "video adapter utilized: " << m_PhysDevice.getProperties().deviceName << " " << to_string(m_PhysDevice.getProperties().deviceType) << "\n";
	auto const prer = m_PhysDevice.enumerateDeviceExtensionProperties();
	for (auto extension_properties : prer)
	{
		for (auto const extensionChar : extension_properties.extensionName)
		{
			std::cerr << extensionChar;
		}
		std::cerr << "\n";
	}
	InitCommandBuffer();
	SetUpSurface();
	InitSwapchain();
	SetUpDepthBuffer();
	SetUpUniformBuffer();
	CreatePipelineLayout();
	InitDescriptorSet();
	InitRenderpass();
}

VkRenderer::~VkRenderer()
{
	m_device.destroyRenderPass(render_pass);
	m_device.freeDescriptorSets(descriptor_pool, descriptor_set);
	m_device.destroyDescriptorPool(descriptor_pool);
	m_device.destroy(pipeline_layout);
	m_device.destroy(descriptor_set_layout);
	m_device.freeMemory(uniform_memory);
	m_device.destroyBuffer(uniform_buffer);
	m_device.destroyImageView(m_depth_view);
	m_device.freeMemory(m_depth_mem);
	m_device.destroyImage(m_depth_image);

	for (auto const& imageView : m_imageViews)
	{
		m_device.destroyImageView(imageView);
	}

	m_device.destroySwapchainKHR(m_swapchain);
	m_instance.destroySurfaceKHR(m_surface);
	m_device.freeCommandBuffers(m_command_pool, m_command_buffer);
	m_device.destroyCommandPool(m_command_pool);
	m_device.destroy();
	m_instance.destroy();
}


uint32_t VkRenderer::FindMemoryType(vk::PhysicalDeviceMemoryProperties const& memProperties, uint32_t typeBits, vk::MemoryPropertyFlags requiredBitmask)
{
	auto typeIndex = uint32_t{};
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) && ((memProperties.memoryTypes[i].propertyFlags & requiredBitmask) == requiredBitmask))
		{
			typeIndex = i;
			break;
		}
		typeBits >>= 1;
	}
	assert(typeIndex != uint32_t(~0));
	return typeIndex;
}


void VkRenderer::CreateInstance()
{
	ExtensionNames = glfwGetRequiredInstanceExtensions(&ExtensionCount);

	vk::ApplicationInfo const application_info
	{
		title,
		VK_MAKE_VERSION(1, 0, 0),
		"Vulkan",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_3
	};

	vk::InstanceCreateInfo const instance_info
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

	m_graphics_queue_family_index = std::distance(m_queue_family_properties.begin(), PropertyIterator);
	assert(m_graphics_queue_family_index < m_queue_family_properties.size());

	constexpr float_t qPriority = 0.0f;
	vk::DeviceQueueCreateInfo DeviceQueueInfo{vk::DeviceQueueCreateFlags{}, static_cast<uint32_t>(m_graphics_queue_family_index), 1, &qPriority};

	auto const deviceExtensionProperties = m_PhysDevice.enumerateDeviceExtensionProperties();

	std::vector<const char*> deviceExtensions{};

	deviceExtensions.emplace_back("VK_KHR_swapchain");
	//deviceExtensions.reserve(deviceExtensionProperties.size());
	
	//for(size_t i{0}; i < deviceExtensionProperties.size(); i++)
	//{
	//	deviceExtensions.push_back(deviceExtensionProperties[i].extensionName);
	//}


	vk::DeviceCreateInfo const deviceInfo
	{
		vk::DeviceCreateFlags(),
		DeviceQueueInfo,
		{},
		deviceExtensions
	};

	m_device = m_PhysDevice.createDevice(deviceInfo);

	std::cerr << "device set up successfully\n";
}

void VkRenderer::InitCommandBuffer()
{
	m_command_pool = m_device.createCommandPool(vk::CommandPoolCreateInfo{ vk::CommandPoolCreateFlags{}, static_cast<uint32_t>(m_graphics_queue_family_index) });

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

	m_available_queue_family_index = m_PhysDevice.getSurfaceSupportKHR(static_cast<uint32_t>(m_graphics_queue_family_index), m_surface) ? m_graphics_queue_family_index : m_queue_family_properties.size();
	if (m_available_queue_family_index == m_queue_family_properties.size())
	{
		for(size_t i{0}; i < m_queue_family_properties.size(); i++)
		{
			if ((m_queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) && m_PhysDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), m_surface))
			{
				m_graphics_queue_family_index = static_cast<uint32_t>(i);
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
	if ((m_graphics_queue_family_index == m_queue_family_properties.size()) || (m_available_queue_family_index == m_queue_family_properties.size()))
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

	vk::PresentModeKHR constexpr swapCurrentMode = vk::PresentModeKHR::eMailbox;

	vk::SurfaceTransformFlagBitsKHR const precedingTransformation = (m_surface_capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		? vk::SurfaceTransformFlagBitsKHR::eIdentity
		: m_surface_capabilities.currentTransform;

	vk::CompositeAlphaFlagBitsKHR const alphaComposite = (m_surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied : (m_surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied : (m_surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
		? vk::CompositeAlphaFlagBitsKHR::eInherit : vk::CompositeAlphaFlagBitsKHR::eOpaque;

	std::array<uint32_t, 2> const queueFamilyIndices{ static_cast<uint32_t>(m_graphics_queue_family_index), static_cast<uint32_t>(m_available_queue_family_index) };


	vk::SwapchainCreateInfoKHR swapchainInfo
	{
		vk::SwapchainCreateFlagsKHR{},
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
	
	if (m_graphics_queue_family_index != m_available_queue_family_index)
	{
		swapchainInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}


	m_swapchain = m_device.createSwapchainKHR(swapchainInfo);
	
	
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

	std::cerr << "swapchain created successfully\n";

}

void VkRenderer::SetUpDepthBuffer()
{
	m_depth_format = vk::Format::eD16Unorm;
	vk::FormatProperties formatProperties = m_PhysDevice.getFormatProperties(m_depth_format);

	if (formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		m_image_tiling = vk::ImageTiling::eLinear;
	else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		m_image_tiling = vk::ImageTiling::eOptimal;
	else
		throw std::runtime_error("Depth attachment is not supported");

	vk::ImageCreateInfo imageInfo
	{
		vk::ImageCreateFlagBits{},
		vk::ImageType::e2D,
		m_depth_format,
		vk::Extent3D(width, height, 1),
		1,
		1,
		vk::SampleCountFlagBits::e1,
		m_image_tiling,
		vk::ImageUsageFlagBits::eDepthStencilAttachment
	};
	m_depth_image = m_device.createImage(imageInfo);

	m_device_memory_properties = m_PhysDevice.getMemoryProperties();
	vk::MemoryRequirements const memRequirements = m_device.getImageMemoryRequirements(m_depth_image);

	//auto typeBits = memRequirements.memoryTypeBits;
	//auto typeIndex = uint32_t{};
	//
	//for (uint32_t i{ 0 }; i < m_device_memory_properties.memoryTypeCount; i++)
	//{
	//	if ((typeBits & 1) && ((m_device_memory_properties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)
	//		== vk::MemoryPropertyFlagBits::eDeviceLocal))
	//	{
	//		typeIndex = i;
	//		break;
	//	}
	//	typeBits >>= 1;
	//}
	//assert(typeIndex != uint32_t(~0));

	auto const typeIndex = FindMemoryType(m_PhysDevice.getMemoryProperties(), memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

	m_depth_mem = m_device.allocateMemory(vk::MemoryAllocateInfo{ memRequirements.size, typeIndex });
	m_device.bindImageMemory(m_depth_image, m_depth_mem, 0);

	m_depth_view = m_device.createImageView(vk::ImageViewCreateInfo{ vk::ImageViewCreateFlags{}, m_depth_image, vk::ImageViewType::e2D, m_depth_format, {},
		{ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }});

	std::cerr << "depth buffer initialized successfully\n";
}

void VkRenderer::SetUpUniformBuffer()
{
	uniform_buffer = m_device.createBuffer(vk::BufferCreateInfo{ vk::BufferCreateFlags{}, sizeof(mvpc), vk::BufferUsageFlagBits::eUniformBuffer });

	vk::MemoryRequirements const memRequirements = m_device.getBufferMemoryRequirements(uniform_buffer);

	uint32_t const typeIndex = FindMemoryType(m_PhysDevice.getMemoryProperties(), memRequirements.memoryTypeBits, 
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	uniform_memory = m_device.allocateMemory(vk::MemoryAllocateInfo{ memRequirements.size, typeIndex });

	auto* dataPtr = static_cast<uint32_t*>(m_device.mapMemory(uniform_memory, 0, memRequirements.size));
	memcpy(dataPtr, &mvpc, sizeof(mvpc));

	m_device.unmapMemory(uniform_memory);

	m_device.bindBufferMemory(uniform_buffer, uniform_memory, 0);

	std::cerr << "uniform buffer set up successfully\n";
}

void VkRenderer::CreatePipelineLayout()
{
	vk::DescriptorSetLayoutBinding descriptorBinding
	{
		{},
		vk::DescriptorType::eUniformBuffer, 1,
		vk::ShaderStageFlagBits::eVertex
	};

	descriptor_set_layout = m_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo{ vk::DescriptorSetLayoutCreateFlags{}, descriptorBinding });

	pipeline_layout = m_device.createPipelineLayout(vk::PipelineLayoutCreateInfo{ vk::PipelineLayoutCreateFlags{}, descriptor_set_layout });

	std::cerr << "Pipeline layout created successfully\n";
}

void VkRenderer::InitDescriptorSet()
{
	vk::DescriptorPoolSize poolSize{ vk::DescriptorType::eUniformBuffer, 1 };

	descriptor_pool = m_device.createDescriptorPool(vk::DescriptorPoolCreateInfo{ vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSize });

	vk::DescriptorSetAllocateInfo const descriptorSetAllocationInfo{ descriptor_pool, descriptor_set_layout };
	descriptor_set = m_device.allocateDescriptorSets(descriptorSetAllocationInfo).front();

	vk::DescriptorBufferInfo descriptorBufferInfo{ uniform_buffer, 0, sizeof(glm::mat4x4) };
	vk::WriteDescriptorSet writeDescriptorSet{ descriptor_set, 0, 0, vk::DescriptorType::eUniformBuffer, {}, descriptorBufferInfo };

	m_device.updateDescriptorSets(writeDescriptorSet, nullptr);

	std::cerr << "descriptor set initialized successfully\n";
}

void VkRenderer::InitRenderpass()
{
	attachment_descriptions[0] = vk::AttachmentDescription
	{
		vk::AttachmentDescriptionFlags{},
		m_format,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::ePresentSrcKHR
	};
	attachment_descriptions[1] = vk::AttachmentDescription
	{
		vk::AttachmentDescriptionFlags{},
		m_depth_format,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eDontCare,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthAttachmentOptimal
	};

	vk::AttachmentReference colorRef{ 0, vk::ImageLayout::eColorAttachmentOptimal };
	vk::AttachmentReference depthRef{ 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };
	vk::SubpassDescription subpass{ vk::SubpassDescriptionFlags{}, vk::PipelineBindPoint::eGraphics, {}, colorRef, {}, &depthRef };

	render_pass = m_device.createRenderPass(vk::RenderPassCreateInfo{ vk::RenderPassCreateFlags{}, attachment_descriptions, subpass });

	std::cerr << "renderpass initialized successfully\n";
}

