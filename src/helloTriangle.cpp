// Vulkan
// Copyright © 2020 otreblan
//
// vulkan-hello is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// vulkan-hello is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with vulkan-hello.  If not, see <http://www.gnu.org/licenses/>.

#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>

#include "config.hpp"
#include "helloTriangle.hpp"
#include "uniformBufferObject.hpp"
#include "vertex.hpp"

void HelloTriangle::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void HelloTriangle::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void HelloTriangle::initVulkan()
{
	createInstance();
#ifdef VK_DEBUG
	setupDebugMessenger();
#endif
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();
}

void HelloTriangle::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		drawFrame();
	}

	device.waitIdle();
}

void HelloTriangle::cleanup()
{
	cleanupSwapChain();

	vkDestroySampler(*device, textureSampler, nullptr);

	vkDestroyImage(*device, textureImage, nullptr);
	vkFreeMemory(*device, textureImageMemory, nullptr);

	vkDestroyDescriptorSetLayout(*device, descriptorSetLayout, nullptr);

	vkDestroyBuffer(*device, indexBuffer, nullptr);
	vkFreeMemory(*device, indexBufferMemory, nullptr);

	vkDestroyBuffer(*device, vertexBuffer, nullptr);
	vkFreeMemory(*device, vertexBufferMemory, nullptr);

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(*device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(*device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(*device, inFlightFences[i], nullptr);
	}
	vkDestroyCommandPool(*device, commandPool, nullptr);
#ifdef VK_DEBUG
	DestroyDebugUtilsMessengerEXT(*instance, debugMessenger, nullptr);
#endif

	glfwDestroyWindow(window);
	glfwTerminate();
}

void HelloTriangle::createInstance()
{
#ifdef VK_DEBUG
	if (!checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}
#endif

	vk::ApplicationInfo appInfo(
		"Hello triangle",
		VK_MAKE_VERSION(0, 0, 0),
		"No engine",
		VK_MAKE_VERSION(0, 0, 0),
		VK_API_VERSION_1_3
	);

	auto glfwExtensions = getRequiredExtensions();

#ifdef VK_DEBUG
	auto debugCreateInfo = populateDebugMessengerCreateInfo();
#endif

	vk::InstanceCreateInfo createInfo(
		{},
		&appInfo,
		validationLayers,
		glfwExtensions,
#ifdef VK_DEBUG
		&debugCreateInfo
#else
		nullptr
#endif
	);

	instance = context.createInstance(createInfo);

	for(const auto& i: vk::enumerateInstanceExtensionProperties())
	{
		std::cout << '\t' << i.extensionName << '\n';
	}
}

std::vector<const char*> HelloTriangle::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef VK_DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	return extensions;
}

#ifdef VK_DEBUG

bool HelloTriangle::checkValidationLayerSupport() {
	for(const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for(const auto& layerProperties : vk::enumerateInstanceLayerProperties())
		{
			if(strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if(!layerFound)
		{
			return false;
		}
	}

	return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangle::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << '\n';
	}

	return VK_FALSE;
}

void HelloTriangle::setupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessengerCreateInfo();

	if (CreateDebugUtilsMessengerEXT(*instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

VkResult HelloTriangle::CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void HelloTriangle::DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}
vk::DebugUtilsMessengerCreateInfoEXT HelloTriangle::populateDebugMessengerCreateInfo()
{
	using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using enum vk::DebugUtilsMessageTypeFlagBitsEXT;

	return {
		{},
		eVerbose | eWarning | eError,
		eGeneral | eValidation | ePerformance,
		debugCallback,
		this
	};
}
#endif

void HelloTriangle::pickPhysicalDevice()
{
	for(const auto& device: instance.enumeratePhysicalDevices())
	{
		if(isDeviceSuitable(*device))
		{
			physicalDevice = device;
			break;
		}
	}

	if(!*physicalDevice)
		throw std::runtime_error("failed to find a suitable GPU!");
}

bool HelloTriangle::isDeviceSuitable(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if(extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() &&
		extensionsSupported &&
		swapChainAdequate &&
		supportedFeatures.samplerAnisotropy
	;
}

QueueFamilyIndices HelloTriangle::findQueueFamilies(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices;

	for(int i = 0; const auto& queueFamily: device.getQueueFamilyProperties())
	{
		if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
			indices.graphicsFamily = i;

		VkBool32 presentSupport = device.getSurfaceSupportKHR(i, *surface);

		if(presentSupport)
			indices.presentFamily = i;

		if(indices.isComplete())
			break;

		i++;
	}


	return indices;
}

void HelloTriangle::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(*physicalDevice);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	float queuePriority[] = {1.0f};

	for(uint32_t queueFamily: uniqueQueueFamilies)
	{
		queueCreateInfos.emplace_back(vk::DeviceQueueCreateInfo({}, queueFamily, queuePriority));
	}

	vk::PhysicalDeviceFeatures deviceFeatures;
	deviceFeatures.samplerAnisotropy = true;

	vk::DeviceCreateInfo createInfo(
		{},
		queueCreateInfos,
		validationLayers,
		deviceExtensions,
		&deviceFeatures
	);

	device = physicalDevice.createDevice(createInfo);

	graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
	presentQueue  = device.getQueue(indices.presentFamily.value(), 0);
}

void HelloTriangle::createSurface()
{
	VkSurfaceKHR _surface;
	if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface!");

	surface = vk::raii::SurfaceKHR(instance, _surface);
}

bool HelloTriangle::checkDeviceExtensionSupport([[maybe_unused]]vk::PhysicalDevice device)
{
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for(const auto& extension: device.enumerateDeviceExtensionProperties())
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

SwapChainSupportDetails HelloTriangle::querySwapChainSupport(vk::PhysicalDevice device)
{
	return {device, *surface};
}

vk::SurfaceFormatKHR HelloTriangle::chooseSwapSurfaceFormat(const std::span<vk::SurfaceFormatKHR> availableFormats)
{
	using enum vk::Format;
	using enum vk::ColorSpaceKHR;

	for(const auto& availableFormat: availableFormats)
	{
		if(availableFormat.format == eB8G8R8A8Srgb && availableFormat.colorSpace == eSrgbNonlinear)
			return availableFormat;
	}

	return availableFormats[0];
}

vk::PresentModeKHR HelloTriangle::chooseSwapPresentMode(const std::span<vk::PresentModeKHR> availablePresentModes)
{
	using enum vk::PresentModeKHR;

	for(const auto& availablePresentMode: availablePresentModes)
	{
		if(availablePresentMode == eMailbox)
			return availablePresentMode;
	}

	return eFifo;
}

vk::Extent2D HelloTriangle::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		return vk::Extent2D(width, height);
	}
}

void HelloTriangle::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(*physicalDevice);

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if(swapChainSupport.capabilities.maxImageCount != 0)
		imageCount = std::min(imageCount, swapChainSupport.capabilities.maxImageCount);

	vk::SwapchainCreateInfoKHR createInfo(
		{},
		*surface,
		imageCount,
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		{},
		{},
		swapChainSupport.capabilities.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		presentMode,
		true,
		nullptr
	);

	QueueFamilyIndices indices = findQueueFamilies(*physicalDevice);

	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if(indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices   = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode      = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices   = nullptr; // Optional
	}

	swapChain            = device.createSwapchainKHR(createInfo);
	swapChainImages      = swapChain.getImages();
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent      = extent;
}

vk::raii::ImageView HelloTriangle::createImageView(vk::Image image, vk::Format format)
{
	vk::ImageViewCreateInfo viewInfo(
		{},
		image,
		vk::ImageViewType::e2D,
		format,
		{},
		vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
	);

	return device.createImageView(viewInfo);
}

void HelloTriangle::createImageViews()
{
	swapChainImageViews.reserve(swapChainImages.size());
	for(auto& image: swapChainImages)
	{
		swapChainImageViews.emplace_back(createImageView(image, swapChainImageFormat));
	}
}

void HelloTriangle::createGraphicsPipeline()
{

	auto vertShaderCode = readFile(shadersDir/"vert.spv");
	auto fragShaderCode = readFile(shadersDir/"frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo
	{
		.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage  = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertShaderModule,
		.pName  = "main"
	};

	VkPipelineShaderStageCreateInfo fragShaderStageInfo
	{
		.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShaderModule,
		.pName  = "main"
	};

	VkPipelineShaderStageCreateInfo shaderStages[] =
	{
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo
	{
		.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount   = 1,
		.pVertexBindingDescriptions      = &Vertex::bindingDescription,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::attributeDescriptions.size()),
		.pVertexAttributeDescriptions    = Vertex::attributeDescriptions.data()
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly
	{
		.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkViewport viewport
	{
		.x        = 0.0f,
		.y        = 0.0f,
		.width    = (float)swapChainExtent.width,
		.height   = (float)swapChainExtent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	VkRect2D scissor
	{
		.offset = {.x = 0, .y = 0},
		.extent = swapChainExtent
	};

	VkPipelineViewportStateCreateInfo viewportState
	{
		.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports    = &viewport,
		.scissorCount  = 1,
		.pScissors     = &scissor
	};

	VkPipelineRasterizationStateCreateInfo rasterizer
	{
		.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable        = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode             = VK_POLYGON_MODE_FILL,
		.cullMode                = VK_CULL_MODE_BACK_BIT,
		// OpenGL -> Vulkan
		.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		//.frontFace               = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable         = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp          = 0.0f,
		.depthBiasSlopeFactor    = 0.0f,
		.lineWidth               = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisampling
	{
		.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable   = VK_FALSE,
		.minSampleShading      = 1.0f,
		.pSampleMask           = nullptr,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable      = VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment
	{
		.blendEnable         = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp        = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp        = VK_BLEND_OP_ADD,
		.colorWriteMask      =
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo colorBlending
	{
		.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable   = VK_FALSE,
		.logicOp         = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments    = &colorBlendAttachment,
		.blendConstants  = {0.0f, 0.0f, 0.0f, 0.0f}
	};

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	[[maybe_unused]]
	VkPipelineDynamicStateCreateInfo dynamicState
	{
		.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates    = dynamicStates
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo
	{
		.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount         = 1,
		.pSetLayouts            = &descriptorSetLayout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges    = nullptr
	};

	if(vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout!");

	VkGraphicsPipelineCreateInfo pipelineInfo
	{
		.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount          = 2,
		.pStages             = shaderStages,
		.pVertexInputState   = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState      = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState   = &multisampling,
		.pDepthStencilState  = nullptr,
		.pColorBlendState    = &colorBlending,
		.pDynamicState       = nullptr,
		.layout              = pipelineLayout,
		.renderPass          = renderPass,
		.subpass             = 0,
		.basePipelineHandle  = VK_NULL_HANDLE,
		.basePipelineIndex   = -1,
	};

	if(vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		throw std::runtime_error("failed to create graphics pipeline!");

	vkDestroyShaderModule(*device, fragShaderModule, nullptr);
	vkDestroyShaderModule(*device, vertShaderModule, nullptr);
}

std::vector<char> HelloTriangle::readFile(const path& filepath)
{
	uintmax_t size = std::filesystem::file_size(filepath);

	std::vector<char> buffer(size);
	std::ifstream file(filepath, std::ios::binary);

	if(!file.is_open())
		throw std::runtime_error("failed to open file!");

	file.read(buffer.data(), size);
	file.close();

	return buffer;
}

VkShaderModule HelloTriangle::createShaderModule(std::span<char> code)
{
	VkShaderModuleCreateInfo createInfo
	{
		.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code.size(),
		.pCode    = reinterpret_cast<const uint32_t*>(code.data())
	};

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(*device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("failed to create shader module!");

	return shaderModule;
}


void HelloTriangle::createRenderPass()
{
	VkAttachmentDescription colorAttachment
	{
		.format         = (VkFormat)swapChainImageFormat,
		.samples        = VK_SAMPLE_COUNT_1_BIT,
		.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};

	VkAttachmentReference colorAttachmentRef
	{
		.attachment = 0,
		.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass
	{
		.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments    = &colorAttachmentRef
	};

	VkSubpassDependency dependency
	{
		.srcSubpass    = VK_SUBPASS_EXTERNAL,
		.dstSubpass    = 0,
		.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
	};

	VkRenderPassCreateInfo renderPassInfo
	{
		.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments    = &colorAttachment,
		.subpassCount    = 1,
		.pSubpasses      = &subpass,
		.dependencyCount = 1,
		.pDependencies   = &dependency
	};

	if(vkCreateRenderPass(*device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("failed to create render pass!");
}

void HelloTriangle::createFramebuffers()
{
	swapChainFramebuffers.reserve(swapChainImageViews.size());

	for(size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		vk::ImageView attachments[1] =
		{
			*swapChainImageViews[i]
		};

		vk::FramebufferCreateInfo framebufferInfo(
			{},
			renderPass,
			attachments,
			swapChainExtent.width,
			swapChainExtent.height,
			1
		);

		swapChainFramebuffers.emplace_back(device.createFramebuffer(framebufferInfo));
	}
}

void HelloTriangle::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(*physicalDevice);

	VkCommandPoolCreateInfo poolInfo
	{
		.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags            = 0,
		.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value()
	};

	if(vkCreateCommandPool(*device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create command pool!");
}

void HelloTriangle::createCommandBuffers()
{
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo
	{
		.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool        = commandPool,
		.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = (uint32_t)commandBuffers.size()
	};

	if(vkAllocateCommandBuffers(*device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate command buffers!");

	for(size_t i = 0; i < commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo
		{
			.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags            = 0,
			.pInheritanceInfo = nullptr
		};

		if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer!");

		VkClearValue clearColor = {.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}};
		VkRenderPassBeginInfo renderPassInfo
		{
			.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass      = renderPass,
			.framebuffer     = *swapChainFramebuffers[i],
			.renderArea
			{
				.offset = {.x = 0, .y = 0},
				.extent = swapChainExtent
			},
			.clearValueCount = 1,
			.pClearValues    = &clearColor
		};

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		VkBuffer vertexBuffers[] = {vertexBuffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);

		if(vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer!");
	}
}

void HelloTriangle::drawFrame()
{
	vkWaitForFences(*device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(*device,
		*swapChain,
		std::numeric_limits<uint64_t>::max(),
		imageAvailableSemaphores[currentFrame],
		VK_NULL_HANDLE,
		&imageIndex
	);

	if(result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("failed to acquire swap chain image!");

	updateUniformBuffer(imageIndex);

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(*device, 1, &imagesInFlight[imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());

	// Mark the image as now being in use by this frame
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submitInfo
	{
		.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount   = 1,
		.pWaitSemaphores      = waitSemaphores,
		.pWaitDstStageMask    = waitStages,
		.commandBufferCount   = 1,
		.pCommandBuffers      = &commandBuffers[imageIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores    = signalSemaphores
	};

	vkResetFences(*device, 1, &inFlightFences[currentFrame]);

	if(vkQueueSubmit(*graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		throw std::runtime_error("failed to submit draw command buffer!");

	VkSwapchainKHR swapChains[] = {*swapChain};

	VkPresentInfoKHR presentInfo
	{
		.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores    = signalSemaphores,
		.swapchainCount     = 1,
		.pSwapchains        = swapChains,
		.pImageIndices      = &imageIndex,
		.pResults           = nullptr,
	};

	result = vkQueuePresentKHR(*presentQueue, &presentInfo);

	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("failed to present swap chain image!");

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangle::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};

	VkFenceCreateInfo fenceInfo
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if(vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(*device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(*device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to create sync objects for a frame!");
	}
}

void HelloTriangle::recreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(*device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
}

void HelloTriangle::cleanupSwapChain()
{
	swapChainFramebuffers.clear();

	vkFreeCommandBuffers(*device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	vkDestroyPipeline(*device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(*device, pipelineLayout, nullptr);
	vkDestroyRenderPass(*device, renderPass, nullptr);

	swapChainImageViews.clear();
	swapChain.clear();

	for(size_t i = 0; i < swapChainImages.size(); i++)
	{
		vkDestroyBuffer(*device, uniformBuffers[i], nullptr);
		vkFreeMemory(*device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(*device, descriptorPool, nullptr);
}

void HelloTriangle::framebufferResizeCallback(GLFWwindow* window, int, int)
{
	auto app = reinterpret_cast<HelloTriangle*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void HelloTriangle::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(decltype(vertices)::value_type) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(*device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(*device, stagingBufferMemory);

	createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer,
		vertexBufferMemory
	);
	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(*device, stagingBuffer, nullptr);
	vkFreeMemory(*device, stagingBufferMemory, nullptr);
}

uint32_t HelloTriangle::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &memProperties);

	for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void HelloTriangle::createBuffer(VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory
)
{
	VkBufferCreateInfo bufferInfo
	{
		.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size        = size,
		.usage       = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	if(vkCreateBuffer(*device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create vertex buffer!");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(*device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo
	{
		.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize  = memRequirements.size,
		.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
	};

	if(vkAllocateMemory(*device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate vertex buffer memory!");

	vkBindBufferMemory(*device, buffer, bufferMemory, 0);
}

void HelloTriangle::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion
	{
		.srcOffset = 0,
		.dstOffset = 0,
		.size      = size
	};

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer HelloTriangle::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo
	{
		.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool        = commandPool,
		.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(*device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void HelloTriangle::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo
	{
		.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers    = &commandBuffer
	};

	vkQueueSubmit(*graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(*graphicsQueue);
	vkFreeCommandBuffers(*device, commandPool, 1, &commandBuffer);
}

void HelloTriangle::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(decltype(indices)::value_type) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(*device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t) bufferSize);
	vkUnmapMemory(*device, stagingBufferMemory);

	createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer,
		indexBufferMemory
	);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(*device, stagingBuffer, nullptr);
	vkFreeMemory(*device, stagingBufferMemory, nullptr);
}

void HelloTriangle::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding
	{
		.binding            = 0,
		.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount    = 1,
		.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
		.pImmutableSamplers = nullptr
	};

	VkDescriptorSetLayoutBinding samplerLayoutBinding
	{
		.binding            = 1,
		.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount    = 1,
		.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pImmutableSamplers = nullptr
	};

	std::array<VkDescriptorSetLayoutBinding, 2> bindings
	{
		uboLayoutBinding,
		samplerLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo
	{
		.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings    = bindings.data()
	};

	if(vkCreateDescriptorSetLayout(*device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
}

void HelloTriangle::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(swapChainImages.size());
	uniformBuffersMemory.resize(swapChainImages.size());

	for(size_t i = 0; i < swapChainImages.size(); i++)
	{
		createBuffer(bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers[i],
			uniformBuffersMemory[i]
		);
	}
}

void HelloTriangle::updateUniformBuffer(uint32_t currentImage)
{
	using namespace std::chrono;
	using namespace glm;

	static auto startTime = high_resolution_clock::now();

	auto currentTime = high_resolution_clock::now();
	float dTime = duration<float, seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo
	{
		.model = rotate(mat4(1.0f), dTime * radians(90.0f), vec3(0.0f, 0.0f, 1.0f)),
		.view  = lookAt(vec3(2.0f, 2.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)),
		.proj  = perspective(radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f)
	};

	// OpenGL -> Vulkan
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(*device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(*device, uniformBuffersMemory[currentImage]);
}

void HelloTriangle::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes
	{
		VkDescriptorPoolSize
		{
			.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = static_cast<uint32_t>(swapChainImages.size())
		},
		VkDescriptorPoolSize
		{
			.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = static_cast<uint32_t>(swapChainImages.size())
		}
	};

	VkDescriptorPoolCreateInfo poolInfo
	{
		.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets       = static_cast<uint32_t>(swapChainImages.size()),
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes    = poolSizes.data()
	};

	if(vkCreateDescriptorPool(*device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor pool!");
}

void HelloTriangle::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo
	{
		.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool     = descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size()),
		.pSetLayouts        = layouts.data()
	};

	descriptorSets.resize(swapChainImages.size());
	if(vkAllocateDescriptorSets(*device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate descriptor sets!");

	for(size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo
		{
			.buffer = uniformBuffers[i],
			.offset = 0,
			.range  = sizeof(UniformBufferObject)
		};

		VkDescriptorImageInfo imageInfo
		{
			.sampler     = textureSampler,
			.imageView   = *textureImageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};


		std::array<VkWriteDescriptorSet, 2> descriptorWrites
		{
			VkWriteDescriptorSet
			{
				.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet           = descriptorSets[i],
				.dstBinding       = 0,
				.dstArrayElement  = 0,
				.descriptorCount  = 1,
				.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pImageInfo       = nullptr,
				.pBufferInfo      = &bufferInfo,
				.pTexelBufferView = nullptr
			},
			VkWriteDescriptorSet
			{
				.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet           = descriptorSets[i],
				.dstBinding       = 1,
				.dstArrayElement  = 0,
				.descriptorCount  = 1,
				.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo       = &imageInfo,
				.pBufferInfo      = nullptr,
				.pTexelBufferView = nullptr
			}
		};

		vkUpdateDescriptorSets(*device,
			static_cast<uint32_t>(descriptorWrites.size()),
			descriptorWrites.data(),
			0,
			nullptr
		);
	}
}

void HelloTriangle::createTextureImage()
{
	path texture = texturesDir/"texture.jpg";

	std::filesystem::file_size(texture);

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texture.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if(!pixels)
		throw std::runtime_error("failed to load texture image!");

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(*device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(*device, stagingBufferMemory);
	stbi_image_free(pixels);

	createImage(texWidth,
		texHeight,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImage,
		textureImageMemory
	);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(*device, stagingBuffer, nullptr);
	vkFreeMemory(*device, stagingBufferMemory, nullptr);
}

void HelloTriangle::createImage(uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkImage& image,
	VkDeviceMemory& imageMemory
)
{
	VkImageCreateInfo imageInfo
	{
		.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.flags         = 0,
		.imageType     = VK_IMAGE_TYPE_2D,
		.format        = format,
		.extent        =
		{
			.width  = width,
			.height = height,
			.depth  = 1,
		},
		.mipLevels     = 1,
		.arrayLayers   = 1,
		.samples       = VK_SAMPLE_COUNT_1_BIT,
		.tiling        = tiling,
		.usage         = usage,
		.sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	if(vkCreateImage(*device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		throw std::runtime_error("failed to create image!");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(*device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo
	{
		.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize  = memRequirements.size,
		.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
	};

	if(vkAllocateMemory(*device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate image memory!");

	vkBindImageMemory(*device, image, imageMemory, 0);
}

void HelloTriangle::transitionImageLayout(VkImage image,
	VkFormat,
	VkImageLayout oldLayout,
	VkImageLayout newLayout
)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier
	{
		.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask       = 0, // TODO
		.dstAccessMask       = 0, // TODO
		.oldLayout           = oldLayout,
		.newLayout           = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image               = image,
		.subresourceRange    =
		{
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel   = 0,
			.levelCount     = 1,
			.baseArrayLayer = 0,
			.layerCount     = 1,
		}
	};

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(commandBuffer);
}

void HelloTriangle::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region
	{
		.bufferOffset      = 0,
		.bufferRowLength   = 0,
		.bufferImageHeight = 0,
		.imageSubresource  =
		{
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel       = 0,
			.baseArrayLayer = 0,
			.layerCount     = 1
		},
		.imageOffset = {.x = 0, .y = 0, .z = 0},
		.imageExtent =
		{
			.width  = width,
			.height = height,
			.depth  = 1
		}
	};

	vkCmdCopyBufferToImage(commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	endSingleTimeCommands(commandBuffer);
}

void HelloTriangle::createTextureImageView()
{
	textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Srgb);
}

void HelloTriangle::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo
	{
		.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter               = VK_FILTER_LINEAR,
		.minFilter               = VK_FILTER_LINEAR,
		.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias              = 0.0f,
		.anisotropyEnable        = VK_TRUE,
		.maxAnisotropy           = 16.0f,
		.compareEnable           = VK_FALSE,
		.compareOp               = VK_COMPARE_OP_ALWAYS,
		.minLod                  = 0.0f,
		.maxLod                  = 0.0f,
		.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE
	};

	if (vkCreateSampler(*device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
		throw std::runtime_error("failed to create texture sampler!");
}
