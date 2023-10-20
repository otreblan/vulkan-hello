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

#include "../config.hpp"
#include "../scene.hpp"
#include "../vertex.hpp"
#include "renderer.hpp"
#include "uniformBufferObject.hpp"

Renderer::Renderer():
	pipeline(*this)
{}

void Renderer::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void Renderer::setActiveScene(Scene* scene)
{
	activeScene = scene;
}

void Renderer::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Renderer::initVulkan()
{
	createInstance();
#ifdef VK_DEBUG
	setupDebugMessenger();
#endif
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	allocator.create(*physicalDevice, *device, *instance);
	createDescriptorSetLayout();
	createCommandPool();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();

	if(activeScene)
		renderables = activeScene->getRenderables();

	// TODO: Upload multiple meshes
	renderables.front().mesh.uploadToGpu(*this);

	pipeline.create();
	createSyncObjects();
}

void Renderer::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		drawFrame();
	}

	device.waitIdle();
}

void Renderer::cleanup()
{
#ifdef VK_DEBUG
	DestroyDebugUtilsMessengerEXT(*instance, debugMessenger, nullptr);
#endif

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Renderer::createInstance()
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

std::vector<const char*> Renderer::getRequiredExtensions()
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

bool Renderer::checkValidationLayerSupport() {
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

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::debugCallback(
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

void Renderer::setupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessengerCreateInfo();

	if (CreateDebugUtilsMessengerEXT(*instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

VkResult Renderer::CreateDebugUtilsMessengerEXT(VkInstance instance,
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

void Renderer::DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}
vk::DebugUtilsMessengerCreateInfoEXT Renderer::populateDebugMessengerCreateInfo()
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

void Renderer::pickPhysicalDevice()
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

bool Renderer::isDeviceSuitable(vk::PhysicalDevice physicalDevice)
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

QueueFamilyIndices Renderer::findQueueFamilies(vk::PhysicalDevice device)
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

void Renderer::createLogicalDevice()
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

void Renderer::createSurface()
{
	VkSurfaceKHR _surface;
	if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface!");

	surface = vk::raii::SurfaceKHR(instance, _surface);
}

bool Renderer::checkDeviceExtensionSupport([[maybe_unused]]vk::PhysicalDevice device)
{
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for(const auto& extension: device.enumerateDeviceExtensionProperties())
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

SwapChainSupportDetails Renderer::querySwapChainSupport(vk::PhysicalDevice device)
{
	return {device, *surface};
}

void Renderer::drawFrame()
{
	[[maybe_unused]]
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

	device.resetFences(*inFlightFences[currentFrame]);

	pipeline.commandBuffers[currentFrame].reset();
	pipeline.recordCommandBuffer(*pipeline.commandBuffers[currentFrame], imageIndex);

	vk::Semaphore waitSemaphores[] = {*imageAvailableSemaphores[currentFrame]};
	vk::Semaphore signalSemaphores[] = {*renderFinishedSemaphores[currentFrame]};
	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

	vk::SubmitInfo submitInfo(
		waitSemaphores,
		waitStages,
		*pipeline.commandBuffers[currentFrame],
		signalSemaphores
	);

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

void Renderer::createSyncObjects()
{
	imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);

	inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

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

void Renderer::framebufferResizeCallback(GLFWwindow* window, int, int)
{
	auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void Renderer::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
	auto singleCommand = makeSingleCommand();

	vk::BufferCopy copyRegion(0, 0, size);

	singleCommand.getBuffer().copyBuffer(srcBuffer, dstBuffer, copyRegion);
}

void Renderer::createDescriptorSetLayout()
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

void Renderer::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(*physicalDevice);

	vk::CommandPoolCreateInfo poolInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		queueFamilyIndices.graphicsFamily.value()
	);

	commandPool = device.createCommandPool(poolInfo);
}

void Renderer::updateUniformBuffer(uint32_t currentImage)
{
	using namespace std::chrono;
	using namespace glm;

	static auto startTime = high_resolution_clock::now();

	auto currentTime = high_resolution_clock::now();
	float dTime = duration<float, seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo
	{
		.model = rotate(mat4(1.0f), dTime * radians(90.0f), vec3(0.0f, 1.0f, 0.0f)),
		.view  = lookAt(vec3(3.0f, 4.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)),
		.proj  = perspective(radians(45.0f), pipeline.swapChainExtent.width / (float) pipeline.swapChainExtent.height, 0.1f, 10.0f)
	};

	// OpenGL -> Vulkan
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(*device, *pipeline.uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(*device, *pipeline.uniformBuffersMemory[currentImage]);
}

void Renderer::createTextureImage()
{
	using enum vk::BufferUsageFlagBits;
	using enum vk::MemoryPropertyFlagBits;

	path texture = texturesDir/"texture.jpg";

	//std::filesystem::file_size(texture);

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texture.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	vk::DeviceSize imageSize = texWidth * texHeight * 4;

	if(!pixels)
		throw std::runtime_error("failed to load texture image!");

	auto [stagingBuffer, stagingBufferMemory] = allocator.createBuffer(
		device,
		imageSize,
		eTransferSrc,
		eHostVisible | eHostCoherent
	);

	void* data = stagingBufferMemory.mapMemory(0, imageSize);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	stagingBufferMemory.unmapMemory();
	stbi_image_free(pixels);

	auto [_textureImage, _textureImageMemory] = createImage(
		texWidth,
		texHeight,
		vk::Format::eR8G8B8A8Srgb,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		eDeviceLocal
	);

	textureImage       = std::move(_textureImage);
	textureImageMemory = std::move(_textureImageMemory);

	transitionImageLayout(*textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	copyBufferToImage(*stagingBuffer, *textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	transitionImageLayout(*textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory> Renderer::createImage(
	uint32_t width,
	uint32_t height,
	vk::Format format,
	vk::ImageTiling tiling,
	vk::ImageUsageFlags usage,
	vk::MemoryPropertyFlags properties
)
{
	vk::raii::Image        image       = nullptr;
	vk::raii::DeviceMemory imageMemory = nullptr;

	vk::ImageCreateInfo imageInfo(
		{},
		vk::ImageType::e2D,
		format,
		{width, height, 1},
		1,
		1,
		vk::SampleCountFlagBits::e1,
		tiling,
		usage,
		vk::SharingMode::eExclusive,
		{},
		vk::ImageLayout::eUndefined
	);

	image = device.createImage(imageInfo);

	vk::MemoryRequirements memRequirements = image.getMemoryRequirements();

	vk::MemoryAllocateInfo allocInfo(
		memRequirements.size,
		allocator.findMemoryType(memRequirements.memoryTypeBits, properties)
	);

	imageMemory = device.allocateMemory(allocInfo);

	image.bindMemory(*imageMemory, 0);

	return {std::move(image), std::move(imageMemory)};
}

void Renderer::transitionImageLayout(vk::Image image,
	vk::Format,
	vk::ImageLayout oldLayout,
	vk::ImageLayout newLayout
)
{
	auto singleCommand = makeSingleCommand();

	vk::ImageMemoryBarrier barrier(
		{}, // TODO
		{}, // TODO
		oldLayout,
		newLayout,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		image,
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0,
			1,
			0,
			1
		)
	);

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if(oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if(oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}
	else if(oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage      = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	singleCommand.getBuffer().pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
}

void Renderer::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
	auto singleCommand = makeSingleCommand();

	vk::BufferImageCopy region(
		0,
		0,
		0,
		vk::ImageSubresourceLayers(
			vk::ImageAspectFlagBits::eColor,
			0,
			0,
			1
		),
		{0, 0, 0},
		{width, height, 1}
	);

	singleCommand.getBuffer().copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
}

vk::raii::ImageView Renderer::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
	vk::ImageViewCreateInfo viewInfo(
		{},
		image,
		vk::ImageViewType::e2D,
		format,
		{},
		vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1)
	);

	return device.createImageView(viewInfo);
}

void Renderer::createTextureImageView()
{
	textureImageView = createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb);
}

void Renderer::createTextureSampler()
{
	vk::SamplerCreateInfo samplerInfo(
		{},
		vk::Filter::eLinear,
		vk::Filter::eLinear,
		vk::SamplerMipmapMode::eLinear,
		vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat,
		0,
		true,
		16,
		false,
		vk::CompareOp::eAlways,
		0,
		0,
		vk::BorderColor::eIntOpaqueBlack,
		false
	);

	textureSampler = device.createSampler(samplerInfo);
}

SingleCommand Renderer::makeSingleCommand()
{
	return {device, *commandPool, *graphicsQueue};
}
