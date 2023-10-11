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

HelloTriangle::HelloTriangle():
	pipeline(*this)
{}

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
	createDescriptorSetLayout();
	createCommandPool();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	createVertexBuffer();
	createIndexBuffer();
	pipeline.create();
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
	vkDestroySampler(*device, textureSampler, nullptr);

	vkDestroyImage(*device, textureImage, nullptr);
	vkFreeMemory(*device, textureImageMemory, nullptr);

	vkDestroyBuffer(*device, indexBuffer, nullptr);
	vkFreeMemory(*device, indexBufferMemory, nullptr);
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

bool HelloTriangle::isDeviceSuitable(vk::PhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

	bool swapChainAdequate = false;
	if(extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	vk::PhysicalDeviceFeatures supportedFeatures = physicalDevice.getFeatures();

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

void HelloTriangle::drawFrame()
{
	auto r = device.waitForFences(*inFlightFences[currentFrame], true, std::numeric_limits<uint64_t>::max());

	auto [result, imageIndex] = pipeline.swapChain.acquireNextImage(
		std::numeric_limits<uint64_t>::max(),
		*imageAvailableSemaphores[currentFrame],
		nullptr
	);

	if(result == vk::Result::eErrorOutOfDateKHR)
	{
		pipeline.recreate();
		return;
	}
	else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		throw std::runtime_error("failed to acquire swap chain image!");

	updateUniformBuffer(imageIndex);

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if(imagesInFlight[imageIndex])
		r = device.waitForFences(imagesInFlight[imageIndex], true, std::numeric_limits<uint64_t>::max());

	// Mark the image as now being in use by this frame
	imagesInFlight[imageIndex] = *inFlightFences[currentFrame];

	vk::Semaphore waitSemaphores[] = {*imageAvailableSemaphores[currentFrame]};
	vk::Semaphore signalSemaphores[] = {*renderFinishedSemaphores[currentFrame]};
	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

	vk::SubmitInfo submitInfo(
		waitSemaphores,
		waitStages,
		*pipeline.commandBuffers[imageIndex],
		signalSemaphores
	);

	device.resetFences(*inFlightFences[currentFrame]);

	graphicsQueue.submit(submitInfo, *inFlightFences[currentFrame]);

	vk::SwapchainKHR swapChains[] = {*pipeline.swapChain};

	vk::PresentInfoKHR presentInfo(
		signalSemaphores,
		swapChains,
		imageIndex
	);

	result = presentQueue.presentKHR(presentInfo);

	if(result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized)
	{
		framebufferResized = false;
		pipeline.recreate();
	}
	else if (result != vk::Result::eSuccess)
		throw std::runtime_error("failed to present swap chain image!");

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangle::createSyncObjects()
{
	imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);

	inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(pipeline.swapChainImages.size(), VK_NULL_HANDLE);

	vk::SemaphoreCreateInfo semaphoreInfo;

	// Start signaled to avoid a deadlock in the first frame.
	vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		imageAvailableSemaphores.emplace_back(device.createSemaphore(semaphoreInfo));
		renderFinishedSemaphores.emplace_back(device.createSemaphore(semaphoreInfo));

		inFlightFences.emplace_back(device.createFence(fenceInfo));
	}
}

void HelloTriangle::framebufferResizeCallback(GLFWwindow* window, int, int)
{
	auto app = reinterpret_cast<HelloTriangle*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void HelloTriangle::createVertexBuffer()
{
	vk::DeviceSize bufferSize = sizeof(decltype(vertices)::value_type) * vertices.size();

	auto [stagingBuffer, stagingBufferMemory] = createBuffer(
		bufferSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	void* data = stagingBufferMemory.mapMemory(0, bufferSize);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	stagingBufferMemory.unmapMemory();


	auto [_vertexBuffer, _vertexBufferMemory] = createBuffer(
		bufferSize,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);

	vertexBuffer       = std::move(_vertexBuffer);
	vertexBufferMemory = std::move(_vertexBufferMemory);

	copyBuffer(*stagingBuffer, *vertexBuffer, bufferSize);
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

uint32_t HelloTriangle::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

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

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> HelloTriangle::createBuffer(
	vk::DeviceSize size,
	vk::BufferUsageFlags usage,
	vk::MemoryPropertyFlags properties
)
{
	vk::raii::Buffer buffer = nullptr;
	vk::raii::DeviceMemory bufferMemory = nullptr;

	vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);

	buffer = device.createBuffer(bufferInfo);

	vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

	vk::MemoryAllocateInfo allocInfo(
		memRequirements.size,
		findMemoryType(memRequirements.memoryTypeBits, properties)
	);

	bufferMemory = device.allocateMemory(allocInfo);

	buffer.bindMemory(*bufferMemory, 0);

	return {std::move(buffer), std::move(bufferMemory)};
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
		.commandPool        = *commandPool,
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
	vkFreeCommandBuffers(*device, *commandPool, 1, &commandBuffer);
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
	vk::DescriptorSetLayoutBinding uboLayoutBinding(
		0,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex,
		nullptr
	);

	vk::DescriptorSetLayoutBinding samplerLayoutBinding(
		1,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	);

	std::array<vk::DescriptorSetLayoutBinding, 2> bindings
	{
		uboLayoutBinding,
		samplerLayoutBinding
	};

	vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings);

	descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
}

void HelloTriangle::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(*physicalDevice);

	vk::CommandPoolCreateInfo poolInfo({}, queueFamilyIndices.graphicsFamily.value());

	commandPool = device.createCommandPool(poolInfo);
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
		.proj  = perspective(radians(45.0f), pipeline.swapChainExtent.width / (float) pipeline.swapChainExtent.height, 0.1f, 10.0f)
	};

	// OpenGL -> Vulkan
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(*device, *pipeline.uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(*device, *pipeline.uniformBuffersMemory[currentImage]);
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
