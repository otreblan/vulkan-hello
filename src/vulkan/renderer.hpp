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

#include "../scene.hpp"
#include "allocator.hpp"
#include "pipeline.hpp"
#include "queueFamilyIndices.hpp"
#include "singleCommand.hpp"
#include "frameData.hpp"

class Renderer
{
	using path = std::filesystem::path;

public:
	Renderer();

	void run();
	void setActiveScene(Scene* scene);

private:
	static const int MAX_FRAMES_IN_FLIGHT = FrameData::MAX_FRAMES_IN_FLIGHT;
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

	Allocator allocator;

	vk::raii::CommandPool commandPool = nullptr;

	vk::raii::ImageView textureImageView = nullptr;
	vk::raii::Sampler   textureSampler   = nullptr;

	vk::raii::Image        textureImage       = nullptr;
	vk::raii::DeviceMemory textureImageMemory = nullptr;

	vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;

	FrameData frameData;
	Pipeline  pipeline;

	bool framebufferResized = false;

	// Non owning reference to the current scene.
	Scene*                  activeScene = nullptr;
	std::vector<Renderable> renderables;

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
	void createCommandPool();

	bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

	vk::raii::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);

	void drawFrame();
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

	void createDescriptorSetLayout();
	void updateUniformBuffer();

	void createTextureImage();
	std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(uint32_t width,
		uint32_t height,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties
	);
	void transitionImageLayout(vk::Image image,
		vk::Format format,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout
	);
	void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
	void createTextureImageView();
	void createTextureSampler();

	friend class Allocator;
	friend class Depth;
	friend class FrameData;
	friend class Mesh;
	friend struct Pipeline;

protected:
	SingleCommand makeSingleCommand();
	vk::Extent2D getWindowSize() const;
};
