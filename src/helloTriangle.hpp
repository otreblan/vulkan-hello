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

#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>
#include <filesystem>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "pipeline.hpp"
#include "queueFamilyIndices.hpp"

class HelloTriangle
{
	using path = std::filesystem::path;

public:
	HelloTriangle();

	void run();

private:
	static const int MAX_FRAMES_IN_FLIGHT = 2;
	GLFWwindow* window;

	const int width = 800;
	const int height = 600;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef VK_DEBUG
	const bool enableValidationLayers = true;
	VkDebugUtilsMessengerEXT debugMessenger;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	bool checkValidationLayerSupport();
	void setupDebugMessenger();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);

	void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);

	vk::DebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo();
#else
	const bool enableValidationLayers = false;
	const std::vector<const char*> validationLayers = {};
#endif

	vk::raii::Context        context;
	vk::raii::Instance       instance       = nullptr;
	vk::raii::PhysicalDevice physicalDevice = nullptr;
	vk::raii::Device         device         = nullptr;
	vk::raii::Queue          graphicsQueue  = nullptr;
	vk::raii::Queue          presentQueue   = nullptr;
	vk::raii::SurfaceKHR     surface        = nullptr;

	Pipeline pipeline;

	vk::raii::CommandPool commandPool = nullptr;
	mutable std::vector<vk::raii::Semaphore> imageAvailableSemaphores;
	mutable std::vector<vk::raii::Semaphore> renderFinishedSemaphores;

	std::vector<vk::raii::Fence> inFlightFences;
	std::vector<vk::Fence> imagesInFlight;

	size_t currentFrame     = 0;
	bool framebufferResized = false;

	vk::raii::Buffer       vertexBuffer       = nullptr;
	vk::raii::DeviceMemory vertexBufferMemory = nullptr;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	vk::raii::ImageView textureImageView = nullptr;
	VkSampler textureSampler;

	void initWindow();
	void initVulkan();
	void mainLoop();
	void cleanup();

	void createInstance();
	std::vector<const char*> getRequiredExtensions();

	void pickPhysicalDevice();
	bool isDeviceSuitable(vk::PhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

	void createLogicalDevice();

	void createSurface();

	bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);
	vk::raii::ImageView createImageView(vk::Image image, vk::Format format);
	void drawFrame();
	void createSyncObjects();
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void createVertexBuffer();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory
	);
	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties
	);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void createIndexBuffer();
	void createDescriptorSetLayout();
	void updateUniformBuffer(uint32_t currentImage);
	void createCommandPool();
	void createTextureImage();
	void createImage(uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory
	);
	void transitionImageLayout(VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout
	);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void createTextureImageView();
	void createTextureSampler();

	friend struct Pipeline;
};
