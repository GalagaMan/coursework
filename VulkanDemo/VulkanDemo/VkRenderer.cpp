#include "VkRenderer.h"


VkRenderer::VkRenderer(Window& window)
	:window(window)
{
	CreateInstance();
	SetUpVkDevice();
	InitCommandBuffer();
	SetUpSurface();
	InitSwapchain();
	SetUpDepthBuffer();
	SetUpUniformBuffer();
	CreatePipelineLayout();
	InitDescriptorSet();
	InitRenderpass();
	InitShaders();
	SetupFrameBuffer();
	SetupVertexBuffer();
	BuildGraphicsPipeline();
	//std::cerr << PhysDevice.getProperties().limits.maxBoundDescriptorSets << "maxBoundDescriptorSets "<< "\n";
	//std::cerr << PhysDevice.getProperties().limits.maxVertexInputBindings << "maxVertexInputBindings" << "\n";
	//std::cerr << PhysDevice.getProperties().limits.maxMemoryAllocationCount << "maxMemoryAllocationCount" << "\n";
	//std::cerr << PhysDevice.getProperties().limits.maxDescriptorSetUniformBuffers << "maxDescriptorSetUniformBuffers" << "\n";
	//std::cerr << PhysDevice.getProperties().limits.maxFramebufferWidth << "maxFramebufferWidth" << "\n";
	image_acquired_sem = logical_device.createSemaphore(vk::SemaphoreCreateInfo{ vk::SemaphoreCreateFlags{} });
	image_has_finished_rendering_sem = logical_device.createSemaphore(vk::SemaphoreCreateInfo{ vk::SemaphoreCreateFlags{} });
	fence = logical_device.createFence(vk::FenceCreateInfo{});
}

VkRenderer::~VkRenderer()
{
	logical_device.destroyPipeline(pipeline);
	logical_device.destroy(fence);
	logical_device.destroySemaphore(image_acquired_sem);
	logical_device.destroySemaphore(image_has_finished_rendering_sem);
	logical_device.freeMemory(vertex_memory);
	logical_device.destroyBuffer(vertex_buffer);

	for (auto const& frameBuffer : framebuffers)
	{
		logical_device.destroyFramebuffer(frameBuffer);
	}

	logical_device.destroyShaderModule(fragment_shader_module);
	logical_device.destroyShaderModule(vertex_shader_module);
	logical_device.destroyRenderPass(render_pass);
	logical_device.freeDescriptorSets(descriptor_pool, descriptor_set);
	logical_device.destroyDescriptorPool(descriptor_pool);
	logical_device.destroy(pipeline_layout);
	logical_device.destroy(descriptor_set_layout);
	logical_device.freeMemory(uniform_memory);
	logical_device.destroyBuffer(uniform_buffer);
	logical_device.destroyImageView(depth_view);
	logical_device.freeMemory(depth_mem);
	logical_device.destroyImage(depth_image);

	for (auto const& imageView : imageViews)
	{
		logical_device.destroyImageView(imageView);
	}

	logical_device.destroySwapchainKHR(swapchain);
	instance.destroySurfaceKHR(surface);
	logical_device.freeCommandBuffers(command_pool, command_buffer);
	logical_device.destroyCommandPool(command_pool);
	logical_device.destroy();
	instance.destroy();
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

void VkRenderer::GetShader(vk::ShaderStageFlagBits const stageBits, std::string& glslShader, std::vector<uint32_t>& spvShader)
{
	std::cerr << "compiling shader " << vk::to_string(stageBits) << "\n";

	TBuiltInResource defaultResource;

	defaultResource.maxLights = 32;
	defaultResource.maxClipPlanes = 6;
	defaultResource.maxTextureUnits = 32;
	defaultResource.maxTextureCoords = 32;
	defaultResource.maxVertexAttribs = 64;
	defaultResource.maxVertexUniformComponents = 4096;
	defaultResource.maxVaryingFloats = 64;
	defaultResource.maxVertexTextureImageUnits = 32;
	defaultResource.maxCombinedTextureImageUnits = 80;
	defaultResource.maxTextureImageUnits = 32;
	defaultResource.maxFragmentUniformComponents = 4096;
	defaultResource.maxDrawBuffers = 32;
	defaultResource.maxVertexUniformVectors = 128;
	defaultResource.maxVaryingVectors = 8;
	defaultResource.maxFragmentUniformVectors = 16;
	defaultResource.maxVertexOutputVectors = 16;
	defaultResource.maxFragmentInputVectors = 15;
	defaultResource.minProgramTexelOffset = -8;
	defaultResource.maxProgramTexelOffset = 7;
	defaultResource.maxClipDistances = 8;
	defaultResource.maxComputeWorkGroupCountX = 65535;
	defaultResource.maxComputeWorkGroupCountY = 65535;
	defaultResource.maxComputeWorkGroupCountZ = 65535;
	defaultResource.maxComputeWorkGroupSizeX = 1024;
	defaultResource.maxComputeWorkGroupSizeY = 1024;
	defaultResource.maxComputeWorkGroupSizeZ = 64;
	defaultResource.maxComputeUniformComponents = 1024;
	defaultResource.maxComputeTextureImageUnits = 16;
	defaultResource.maxComputeImageUniforms = 8;
	defaultResource.maxComputeAtomicCounters = 8;
	defaultResource.maxComputeAtomicCounterBuffers = 1;
	defaultResource.maxVaryingComponents = 60;
	defaultResource.maxVertexOutputComponents = 64;
	defaultResource.maxGeometryInputComponents = 64;
	defaultResource.maxGeometryOutputComponents = 128;
	defaultResource.maxFragmentInputComponents = 128;
	defaultResource.maxImageUnits = 8;
	defaultResource.maxCombinedImageUnitsAndFragmentOutputs = 8;
	defaultResource.maxCombinedShaderOutputResources = 8;
	defaultResource.maxImageSamples = 0;
	defaultResource.maxVertexImageUniforms = 0;
	defaultResource.maxTessControlImageUniforms = 0;
	defaultResource.maxTessEvaluationImageUniforms = 0;
	defaultResource.maxGeometryImageUniforms = 0;
	defaultResource.maxFragmentImageUniforms = 8;
	defaultResource.maxCombinedImageUniforms = 8;
	defaultResource.maxGeometryTextureImageUnits = 16;
	defaultResource.maxGeometryOutputVertices = 256;
	defaultResource.maxGeometryTotalOutputComponents = 1024;
	defaultResource.maxGeometryUniformComponents = 1024;
	defaultResource.maxGeometryVaryingComponents = 64;
	defaultResource.maxTessControlInputComponents = 128;
	defaultResource.maxTessControlOutputComponents = 128;
	defaultResource.maxTessControlTextureImageUnits = 16;
	defaultResource.maxTessControlUniformComponents = 1024;
	defaultResource.maxTessControlTotalOutputComponents = 4096;
	defaultResource.maxTessEvaluationInputComponents = 128;
	defaultResource.maxTessEvaluationOutputComponents = 128;
	defaultResource.maxTessEvaluationTextureImageUnits = 16;
	defaultResource.maxTessEvaluationUniformComponents = 1024;
	defaultResource.maxTessPatchComponents = 120;
	defaultResource.maxPatchVertices = 32;
	defaultResource.maxTessGenLevel = 64;
	defaultResource.maxViewports = 16;
	defaultResource.maxVertexAtomicCounters = 0;
	defaultResource.maxTessControlAtomicCounters = 0;
	defaultResource.maxTessEvaluationAtomicCounters = 0;
	defaultResource.maxGeometryAtomicCounters = 0;
	defaultResource.maxFragmentAtomicCounters = 8;
	defaultResource.maxCombinedAtomicCounters = 8;
	defaultResource.maxAtomicCounterBindings = 1;
	defaultResource.maxVertexAtomicCounterBuffers = 0;
	defaultResource.maxTessControlAtomicCounterBuffers = 0;
	defaultResource.maxTessEvaluationAtomicCounterBuffers = 0;
	defaultResource.maxGeometryAtomicCounterBuffers = 0;
	defaultResource.maxFragmentAtomicCounterBuffers = 1;
	defaultResource.maxCombinedAtomicCounterBuffers = 1;
	defaultResource.maxAtomicCounterBufferSize = 16384;
	defaultResource.maxTransformFeedbackBuffers = 4;
	defaultResource.maxTransformFeedbackInterleavedComponents = 64;
	defaultResource.maxCullDistances = 8;
	defaultResource.maxCombinedClipAndCullDistances = 8;
	defaultResource.maxSamples = 4;
	defaultResource.maxMeshOutputVerticesNV = 256;
	defaultResource.maxMeshOutputPrimitivesNV = 512;
	defaultResource.maxMeshWorkGroupSizeX_NV = 32;
	defaultResource.maxMeshWorkGroupSizeY_NV = 1;
	defaultResource.maxMeshWorkGroupSizeZ_NV = 1;
	defaultResource.maxTaskWorkGroupSizeX_NV = 32;
	defaultResource.maxTaskWorkGroupSizeY_NV = 1;
	defaultResource.maxTaskWorkGroupSizeZ_NV = 1;
	defaultResource.maxMeshViewCountNV = 4;

	defaultResource.limits.nonInductiveForLoops = 1;
	defaultResource.limits.whileLoops = 1;
	defaultResource.limits.doWhileLoops = 1;
	defaultResource.limits.generalUniformIndexing = 1;
	defaultResource.limits.generalAttributeMatrixVectorIndexing = 1;
	defaultResource.limits.generalVaryingIndexing = 1;
	defaultResource.limits.generalSamplerIndexing = 1;
	defaultResource.limits.generalVariableIndexing = 1;
	defaultResource.limits.generalConstantMatrixVectorIndexing = 1;

	EShLanguage translation{};
	switch (stageBits)
	{
	case vk::ShaderStageFlagBits::eVertex: translation = EShLangVertex;
		break;
	case vk::ShaderStageFlagBits::eGeometry: translation = EShLangGeometry;
		break;
	case vk::ShaderStageFlagBits::eFragment: translation = EShLangFragment;
		break;
	default: std::cerr << "shader not supported";
	}

	const char* shaderStrings[1];
	shaderStrings[0] = glslShader.data();

	glslang::TShader shader(translation);
	shader.setStrings(shaderStrings, 1);

	auto messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

	if (!shader.parse(&defaultResource, 400, false, messages))
	{
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		throw std::runtime_error("couldn't parse the shaders");
	}

	glslang::TProgram program;
	program.addShader(&shader);

	if (!program.link(messages))
	{
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		std::cout << std::flush;
		throw std::runtime_error("couldn't link the shader to the program: " + vk::to_string(stageBits));
	}

	glslang::GlslangToSpv(*program.getIntermediate(translation), spvShader);

	std::cerr << "compiling done\n";
}

void VkRenderer::Draw()
{
	
	vk::ResultValue<uint32_t> currentBuffer = logical_device.acquireNextImageKHR(swapchain, TIMEOUT, image_acquired_sem, nullptr);
	assert(currentBuffer.result == vk::Result::eSuccess);
	assert(currentBuffer.value < framebuffers.size());


	clear_values[0].color = vk::ClearColorValue(std::array<float, 4>({ {0.2f, 0.2f, 0.2f, 0.2f} }));
	clear_values[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);


	auto const queue = logical_device.getQueue(graphics_queue_family_index, available_queue_family_index);

	
	do
	{
		command_buffer.begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlags{} });

		vk::RenderPassBeginInfo renderpassBeginInfo
		{
			render_pass,
			framebuffers[currentBuffer.value],
			vk::Rect2D{vk::Offset2D{0,0}, vk::Extent2D{window.Width(), window.Height()}},
			clear_values
		};

		command_buffer.beginRenderPass(renderpassBeginInfo, vk::SubpassContents::eInline);

		command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
		command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout, 0, descriptor_set, nullptr);

		command_buffer.bindVertexBuffers(0, vertex_buffer, { 0 });

		command_buffer.setViewport(0, vk::Viewport{ 0.0f, 0.0f, static_cast<float>(window.Width()), static_cast<float>(window.Height()), 0.0f, 1.0f});

		command_buffer.setScissor(0, vk::Rect2D{ vk::Offset2D{0, 0}, vk::Extent2D{window.Width(), window.Height()} });

		command_buffer.draw(12*3, 1, 0, 0);

		command_buffer.endRenderPass();
		command_buffer.end();


		vk::PipelineStageFlags waitDestinationStageMask{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
		vk::SubmitInfo submitionInfo{ image_acquired_sem, waitDestinationStageMask, command_buffer };

		
			//vk::SubmitInfo(0, nullptr, nullptr, 1, &command_buffer), fence);
		queue.submit(submitionInfo, fence);
	}
	while (vk::Result::eTimeout == logical_device.waitForFences(fence, VK_TRUE, TIMEOUT));

	vk::Result result = queue.presentKHR(vk::PresentInfoKHR{ image_has_finished_rendering_sem, swapchain, currentBuffer.value });
	switch (result)
	{
	case vk::Result::eSuccess:
		break;
	case vk::Result::eSuboptimalKHR:
		std::cerr << "suboptimal present pass happened\n";
		break;
	}

	logical_device.resetFences(fence);
	//logical_device.destroySemaphore(image_acquired_sem);

	logical_device.waitIdle();
}



void VkRenderer::CreateInstance()
{
	ExtensionNames = glfwGetRequiredInstanceExtensions(&ExtensionCount);

	vk::ApplicationInfo const application_info
	{
		window.Title(),
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

	instance = vk::createInstance(instance_info);
	std::cerr << "instance created successfully\n";
}

vk::DeviceQueueCreateInfo VkRenderer::FindQueue(vk::PhysicalDevice& device,vk::QueueFlagBits queueBits, std::vector<float_t>&& priorities)
{
	auto queueFamilyProperties = device.getQueueFamilyProperties();
	auto const propertyIterator = std::find_if(queueFamilyProperties.begin(),
		queueFamilyProperties.end(), [queueBits](vk::QueueFamilyProperties const& queue_family_properties)
		{return queue_family_properties.queueFlags & queueBits; });
	auto graphicsQueueFamilyIndex = std::distance(queueFamilyProperties.begin(), propertyIterator);

	return vk::DeviceQueueCreateInfo{ vk::DeviceQueueCreateFlags{}, static_cast<uint32_t>(graphicsQueueFamilyIndex), priorities };
}

void VkRenderer::SetUpVkDevice()
{
	PhysDevice = instance.enumeratePhysicalDevices().back();

	//queue_family_properties = PhysDevice.getQueueFamilyProperties();
	//
	//for (auto queueprops : PhysDevice.getQueueFamilyProperties())
	//{
	//	std::cerr << to_string(queueprops.queueFlags) << " " << queueprops.queueCount << "\n";
	//}
	//
	//auto const PropertyIterator = std::find_if(queue_family_properties.begin(),
	//	queue_family_properties.end(), [](vk::QueueFamilyProperties const& queue_family_properties)
	//	{return queue_family_properties.queueFlags & vk::QueueFlagBits::eGraphics; });
	//
	//graphics_queue_family_index = std::distance(queue_family_properties.begin(), PropertyIterator);
	//assert(graphics_queue_family_index < queue_family_properties.size());
	//
	//constexpr float_t qPriority = 0.0f;
	//vk::DeviceQueueCreateInfo DeviceQueueInfo{vk::DeviceQueueCreateFlags{}, static_cast<uint32_t>(graphics_queue_family_index), 1, &qPriority};

	auto const deviceExtensionProperties = PhysDevice.enumerateDeviceExtensionProperties();

	std::vector<const char*> deviceExtensions{};

	deviceExtensions.emplace_back("VK_KHR_swapchain");

	std::vector<vk::DeviceQueueCreateInfo> queueInfo;
	queueInfo.emplace_back(FindQueue(PhysDevice, vk::QueueFlagBits::eGraphics, { 0.0f }));

	vk::DeviceCreateInfo const deviceInfo
	{
		vk::DeviceCreateFlags(),
		queueInfo,
		{},
		deviceExtensions
	};

	logical_device = PhysDevice.createDevice(deviceInfo);

	std::cerr << "video adapter utilized: " << PhysDevice.getProperties().deviceName << " " << to_string(PhysDevice.getProperties().deviceType) << "\n";
	//auto const prer = PhysDevice.enumerateDeviceExtensionProperties();
	//for (auto extension_properties : prer)
	//{
	//	for (auto const extensionChar : extension_properties.extensionName)
	//	{
	//		std::cerr << extensionChar;
	//	}
	//	std::cerr << "\n";
	//}

	std::cerr << "logical_device set up successfully\n";
}

void VkRenderer::InitCommandBuffer()
{
	command_pool = logical_device.createCommandPool(vk::CommandPoolCreateInfo{ vk::CommandPoolCreateFlags{}, static_cast<uint32_t>(graphics_queue_family_index) });

	command_buffer = logical_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{ command_pool, vk::CommandBufferLevel::ePrimary, 1 }).front();

	std::cerr << "command buffer initialized successfully\n";
}

void VkRenderer::SetUpSurface()
{
	VkSurfaceKHR _surface;
	VkResult const error = glfwCreateWindowSurface(instance, window.window_, nullptr, &_surface);
	if (error != VK_SUCCESS)
		throw std::runtime_error("failed to create vulkan rendering surface");
	surface = vk::SurfaceKHR{ _surface };

	available_queue_family_index = PhysDevice.getSurfaceSupportKHR(static_cast<uint32_t>(graphics_queue_family_index), surface)
	? graphics_queue_family_index : queue_family_properties.size();
	if (available_queue_family_index == queue_family_properties.size())
	{
		for(size_t i{0}; i < queue_family_properties.size(); i++)
		{
			if ((queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) && PhysDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
			{
				graphics_queue_family_index = static_cast<uint32_t>(i);
				available_queue_family_index = static_cast<uint32_t>(i);
				break;
			}
		}
	}
	if(available_queue_family_index == queue_family_properties.size())
	{
		for(size_t i{0}; i < queue_family_properties.size(); i++)
		{
			if (PhysDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
			{
				available_queue_family_index = static_cast<uint32_t>(i);
				break;
			}
		}
	}
	if ((graphics_queue_family_index == queue_family_properties.size()) || (available_queue_family_index == queue_family_properties.size()))
		throw std::runtime_error("no queue suitable for graphics found");

	std::vector<vk::SurfaceFormatKHR> const surfaceFormats = PhysDevice.getSurfaceFormatsKHR(surface);
	if (surfaceFormats.empty())
		throw std::exception("no vulkan rendering surface formats found");
	format = (surfaceFormats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : surfaceFormats[0].format;

	surface_capabilities = PhysDevice.getSurfaceCapabilitiesKHR(surface);

	std::cerr << "vulkan rendering surface set up successfully\n";
}

void VkRenderer::InitSwapchain()
{
	vk::Extent2D SwapExtent;
	if (surface_capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
	{
		SwapExtent.width = (window.Width(), surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
		SwapExtent.height = (window.Height(), surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
	}
	else
	{
		SwapExtent = surface_capabilities.currentExtent;
	}

	vk::PresentModeKHR constexpr swapCurrentMode = vk::PresentModeKHR::eImmediate;

	vk::SurfaceTransformFlagBitsKHR const precedingTransformation = (surface_capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		? vk::SurfaceTransformFlagBitsKHR::eIdentity
		: surface_capabilities.currentTransform;

	vk::CompositeAlphaFlagBitsKHR const alphaComposite = (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied : (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied)
		? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied : (surface_capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)
		? vk::CompositeAlphaFlagBitsKHR::eInherit : vk::CompositeAlphaFlagBitsKHR::eOpaque;

	std::array<uint32_t, 2> const queueFamilyIndices{ static_cast<uint32_t>(graphics_queue_family_index), static_cast<uint32_t>(available_queue_family_index) };


	vk::SwapchainCreateInfoKHR swapchainInfo
	{
		vk::SwapchainCreateFlagsKHR{},
		surface,
		surface_capabilities.minImageCount,
		format,
		vk::ColorSpaceKHR::eSrgbNonlinear,
		SwapExtent,
		1, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
		vk::SharingMode::eExclusive,
		{},
		precedingTransformation,
		alphaComposite,
		swapCurrentMode,
		VK_TRUE,
		nullptr,
	};
	
	if (graphics_queue_family_index != available_queue_family_index)
	{
		swapchainInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}


	swapchain = logical_device.createSwapchainKHR(swapchainInfo);
	
	
	std::vector<vk::Image> swapchainImages = logical_device.getSwapchainImagesKHR(swapchain);

	
	imageViews.reserve(swapchainImages.size());

	vk::ImageViewCreateInfo imageViewInfo
	{
		vk::ImageViewCreateFlags{},
		{},
		vk::ImageViewType::e2D,
		format,
		{},
		{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
	};

	for(auto const & image : swapchainImages)
	{
		imageViewInfo.image = image;
		imageViews.push_back(logical_device.createImageView(imageViewInfo));
	}

	std::cerr << "swapchain created successfully\n";

}

void VkRenderer::SetUpDepthBuffer()
{
	depth_format = vk::Format::eD16Unorm;
	vk::FormatProperties const formatProperties = PhysDevice.getFormatProperties(depth_format);

	if (formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		image_tiling = vk::ImageTiling::eLinear;
	else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		image_tiling = vk::ImageTiling::eOptimal;
	else
		throw std::runtime_error("Depth attachment is not supported");

	vk::ImageCreateInfo const imageInfo
	{
		vk::ImageCreateFlagBits{},
		vk::ImageType::e2D,
		depth_format,
		vk::Extent3D(window.Width(), window.Height(), 1),
		1,
		1,
		vk::SampleCountFlagBits::e1,
		image_tiling,
		vk::ImageUsageFlagBits::eDepthStencilAttachment
	};
	depth_image = logical_device.createImage(imageInfo);

	device_memory_properties = PhysDevice.getMemoryProperties();
	vk::MemoryRequirements const memRequirements = logical_device.getImageMemoryRequirements(depth_image);

	//auto typeBits = memRequirements.memoryTypeBits;
	//auto typeIndex = uint32_t{};
	//
	//for (uint32_t i{ 0 }; i < device_memory_properties.memoryTypeCount; i++)
	//{
	//	if ((typeBits & 1) && ((device_memory_properties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)
	//		== vk::MemoryPropertyFlagBits::eDeviceLocal))
	//	{
	//		typeIndex = i;
	//		break;
	//	}
	//	typeBits >>= 1;
	//}
	//assert(typeIndex != uint32_t(~0));

	auto const typeIndex = FindMemoryType(PhysDevice.getMemoryProperties(), memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

	depth_mem = logical_device.allocateMemory(vk::MemoryAllocateInfo{ memRequirements.size, typeIndex });
	logical_device.bindImageMemory(depth_image, depth_mem, 0);

	depth_view = logical_device.createImageView(vk::ImageViewCreateInfo{ vk::ImageViewCreateFlags{}, depth_image, vk::ImageViewType::e2D, depth_format, {},
		{ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 }});

	std::cerr << "depth buffer initialized successfully\n";
}

void VkRenderer::SetUpUniformBuffer()
{
	uniform_buffer = logical_device.createBuffer(vk::BufferCreateInfo{ vk::BufferCreateFlags{}, sizeof(mvpc), vk::BufferUsageFlagBits::eUniformBuffer });

	vk::MemoryRequirements const memRequirements = logical_device.getBufferMemoryRequirements(uniform_buffer);

	uint32_t const typeIndex = FindMemoryType(PhysDevice.getMemoryProperties(), memRequirements.memoryTypeBits, 
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	uniform_memory = logical_device.allocateMemory(vk::MemoryAllocateInfo{ memRequirements.size, typeIndex });

	auto* dataPtr = static_cast<uint32_t*>(logical_device.mapMemory(uniform_memory, 0, memRequirements.size));
	memcpy(dataPtr, &mvpc, sizeof(mvpc));

	logical_device.unmapMemory(uniform_memory);

	logical_device.bindBufferMemory(uniform_buffer, uniform_memory, 0);

	std::cerr << "uniform buffer set up successfully\n";
}

void VkRenderer::CreatePipelineLayout()
{
	vk::DescriptorSetLayoutBinding descriptorBinding
	{
		{},
		vk::DescriptorType::eUniformBuffer, 1,
		vk::ShaderStageFlagBits::eVertex,
	};

	//std::array<vk::DescriptorSetLayoutBinding, 2> descriptorSetLayoutBinding
	//{
	//	vk::DescriptorSetLayoutBinding
	//	{
	//		{}, vk::DescriptorType::eUniformBuffer, 1,
	//		vk::ShaderStageFlagBits::eVertex
	//	},
	//	vk::DescriptorSetLayoutBinding
	//	{
	//		{}, vk::DescriptorType::eUniformBuffer, 1,
	//		vk::ShaderStageFlagBits::eFragment
	//	}
	//};


	descriptor_set_layout = logical_device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo{ vk::DescriptorSetLayoutCreateFlags{}, descriptorBinding });

	pipeline_layout = logical_device.createPipelineLayout(vk::PipelineLayoutCreateInfo{ vk::PipelineLayoutCreateFlags{}, descriptor_set_layout });

	std::cerr << "Pipeline layout created successfully\n";
}

void VkRenderer::InitDescriptorSet()
{
	vk::DescriptorPoolSize poolSize{ vk::DescriptorType::eUniformBuffer, 2 };

	descriptor_pool = logical_device.createDescriptorPool(vk::DescriptorPoolCreateInfo{ vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSize });

	vk::DescriptorSetAllocateInfo const descriptorSetAllocationInfo{ descriptor_pool, descriptor_set_layout };
	descriptor_set = logical_device.allocateDescriptorSets(descriptorSetAllocationInfo).front();

	vk::DescriptorBufferInfo descriptorBufferInfo{ uniform_buffer, 0, sizeof(glm::mat4x4) };

	vk::WriteDescriptorSet writeDescriptorSet{ descriptor_set, 0, 0, vk::DescriptorType::eUniformBuffer, {}, descriptorBufferInfo };

	logical_device.updateDescriptorSets(writeDescriptorSet, nullptr);

	std::cerr << "descriptor set initialized successfully\n";
}

void VkRenderer::InitRenderpass()
{
	attachment_descriptions[0] = vk::AttachmentDescription
	{
		vk::AttachmentDescriptionFlags{},
		format,
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
		depth_format,
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

	render_pass = logical_device.createRenderPass(vk::RenderPassCreateInfo{ vk::RenderPassCreateFlags{}, attachment_descriptions, subpass });

	std::cerr << "renderpass initialized successfully\n";
}

void VkRenderer::InitShaders()
{
	glslang::InitializeProcess();

	std::string vertexShaderData = FileManager::ReadFile(".\\smp_vertex_shader.vert");

	std::vector<uint32_t> vertexShaderSPV;
	GetShader(vk::ShaderStageFlagBits::eVertex, vertexShaderData, vertexShaderSPV);

	vk::ShaderModuleCreateInfo const vertexShaderModuleInfo{ vk::ShaderModuleCreateFlags{}, vertexShaderSPV };

	vertex_shader_module = logical_device.createShaderModule(vertexShaderModuleInfo);

	std::string fragmentShaderData = FileManager::ReadFile(".\\smp_fragment_shader.frag");

	std::vector<uint32_t> fragmentShaderSpv;
	GetShader(vk::ShaderStageFlagBits::eFragment, fragmentShaderData, fragmentShaderSpv);

	vk::ShaderModuleCreateInfo const fragmentShaderModuleInfo{ vk::ShaderModuleCreateFlags{}, fragmentShaderSpv };

	fragment_shader_module = logical_device.createShaderModule(fragmentShaderModuleInfo);

	glslang::FinalizeProcess();

	std::cerr << "shaders initialized successfully\n";
}

void VkRenderer::SetupFrameBuffer()
{
	attachments[1] = depth_view;

	vk::FramebufferCreateInfo framebufferInfo
	{
		vk::FramebufferCreateFlags{},
		render_pass,
		attachments,
		window.Width(),
		window.Height(),
		1
	};

	framebuffers.reserve(imageViews.size());

	for (auto const& imageView : imageViews)
	{
		attachments[0] = imageView;
		framebuffers.push_back(logical_device.createFramebuffer(framebufferInfo));
	}
}

void VkRenderer::SetupVertexBuffer()
{
	vertex_buffer = logical_device.createBuffer(vk::BufferCreateInfo{ vk::BufferCreateFlags{}, sizeof(coloredCubeData), vk::BufferUsageFlagBits::eVertexBuffer });

	vk::MemoryRequirements memRequirments = logical_device.getBufferMemoryRequirements(vertex_buffer);
	uint32_t const memTypeIndex = FindMemoryType(PhysDevice.getMemoryProperties(), memRequirments.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	vertex_memory = logical_device.allocateMemory(vk::MemoryAllocateInfo{memRequirments.size, memTypeIndex});

	auto const pData = static_cast<uint32_t*>(logical_device.mapMemory(vertex_memory, 0, memRequirments.size));
	memcpy(pData, coloredCubeData, sizeof(coloredCubeData));
	logical_device.unmapMemory(vertex_memory);

	logical_device.bindBufferMemory(vertex_buffer, vertex_memory, 0);

	std::cerr << "vertex buffer set up successfully\n";
}

void VkRenderer::BuildGraphicsPipeline()
{
	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageInfos
	{
		vk::PipelineShaderStageCreateInfo{vk::PipelineShaderStageCreateFlags{}, vk::ShaderStageFlagBits::eVertex, vertex_shader_module, "main"},
		vk::PipelineShaderStageCreateInfo{vk::PipelineShaderStageCreateFlagBits{}, vk::ShaderStageFlagBits::eFragment, fragment_shader_module, "main"}
	};

	vk::VertexInputBindingDescription vertexInputBindingDescription{ 0, sizeof(coloredCubeData[0])};
	std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttributeDescriptions
	{
		vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32A32Sfloat, 0},
		vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32A32Sfloat, sizeof(coloredCubeData[0].a)*4}
	};

	vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateInfo{ vk::PipelineVertexInputStateCreateFlags{}, vertexInputBindingDescription, vertexInputAttributeDescriptions };

	vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyInfo{ vk::PipelineInputAssemblyStateCreateFlags{}, vk::PrimitiveTopology::eTriangleList };

	vk::PipelineViewportStateCreateInfo pipelineViewportInfo{ vk::PipelineViewportStateCreateFlags{}, 1, nullptr, 1, nullptr };

	vk::PipelineRasterizationStateCreateInfo rasterizerStateInfo
	{
		vk::PipelineRasterizationStateCreateFlagBits{},
		VK_FALSE,
		VK_FALSE,
		vk::PolygonMode::eFill,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eClockwise,
		false,
		0.0f,
		0.0f,
		0.0f,
		1.0f
	};

	vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateInfo{ vk::PipelineMultisampleStateCreateFlags(),
		vk::SampleCountFlagBits::e1};

	vk::StencilOpState stencilOpState{ vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways };

	vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilState
	{
		vk::PipelineDepthStencilStateCreateFlags{},
		VK_TRUE,
		VK_TRUE,
		vk::CompareOp::eLessOrEqual,
		VK_FALSE,
		VK_FALSE,
		stencilOpState,
		stencilOpState
	};

	vk::ColorComponentFlags colorFlags{ vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA };

	vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState
	{
		VK_TRUE,
		vk::BlendFactor::eSrcAlpha,
		vk::BlendFactor::eSrcAlpha,
		vk::BlendOp::eAdd,
		vk::BlendFactor::eZero,
		vk::BlendFactor::eZero,
		vk::BlendOp::eAdd,
		colorFlags
	};

	vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateInfo
	{
		vk::PipelineColorBlendStateCreateFlags{},
		VK_FALSE,
		vk::LogicOp::eNoOp,
		pipelineColorBlendAttachmentState,
		{1.0f, 1.0f, 1.0f, 1.0f}
	};

	std::array<vk::DynamicState, 2> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	vk::PipelineDynamicStateCreateInfo pipelineDynamicStateInfo{vk::PipelineDynamicStateCreateFlags{}, dynamicStates};

	vk::GraphicsPipelineCreateInfo graphicsPipelineInfo
	{
		vk::PipelineCreateFlags{},
		pipelineShaderStageInfos,
		&pipelineVertexInputStateInfo,
		&pipelineInputAssemblyInfo,
		nullptr,
		&pipelineViewportInfo,
		&rasterizerStateInfo,
		&pipelineMultisampleStateInfo,
		&pipelineDepthStencilState,
		&pipelineColorBlendStateInfo,
		&pipelineDynamicStateInfo,
		pipeline_layout,
		render_pass
	};

	vk::Result result;

	std::tie(result, pipeline) = logical_device.createGraphicsPipeline(nullptr, graphicsPipelineInfo);

	switch (result)
	{
	case vk::Result::eSuccess:
		std::cerr << "pipeline built successfully\n";
		break;
	case vk::Result::ePipelineCompileRequiredEXT:
		break;
	default: throw std::runtime_error{ "pipeline could not be created" };
	}
}



vertexBufferMemoryPair VkRenderer::LoadMesh(Mesh const& mesh)
{
	vertex_buffers.push_back(logical_device.createBuffer(vk::BufferCreateInfo{vk::BufferCreateFlags{}, sizeof(mesh), vk::BufferUsageFlagBits::eVertexBuffer}));

	vk::MemoryRequirements memRequirments = logical_device.getBufferMemoryRequirements(vertex_buffers.back());
	uint32_t const memTypeIndex = FindMemoryType(PhysDevice.getMemoryProperties(), memRequirments.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	vertex_memory_segments.push_back(logical_device.allocateMemory(vk::MemoryAllocateInfo{ memRequirments.size, memTypeIndex }));
	auto const pData = static_cast<uint32_t*>(logical_device.mapMemory(vertex_memory_segments.back(), 0, memRequirments.size));
	memcpy(pData, mesh.vertices.data(), sizeof(mesh));
	logical_device.unmapMemory(vertex_memory_segments.back());

	logical_device.bindBufferMemory(vertex_buffers.back(), vertex_memory_segments.back(), 0);

	auto s = *(--vertex_buffers.end());
	auto x = *(--vertex_memory_segments.end());

	return {--vertex_buffers.end(), --vertex_memory_segments.end()};
}

void VkRenderer::UnloadMesh(vertexBufferMemoryPair const& buffer)
{
	logical_device.freeMemory(*buffer.second);
	logical_device.destroyBuffer(*buffer.first);
	vertex_memory_segments.erase(buffer.second);
	vertex_buffers.erase(buffer.first);
	
}

