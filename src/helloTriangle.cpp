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

//#include <vulkan/vulkan.h>

#include <stdexcept>
#include <iostream>
#include <vector>

#include <helloTriangle.hpp>

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
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, heigth, "Vulkan", glfwGetPrimaryMonitor(), nullptr);
}

void HelloTriangle::initVulkan()
{
	createInstance();
}

void HelloTriangle::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

void HelloTriangle::cleanup()
{
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);
	glfwTerminate();
}

void HelloTriangle::createInstance()
{
	VkApplicationInfo appInfo
	{
		.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName   = "Hello triangle",
		.applicationVersion = VK_MAKE_VERSION(0, 0, 0),
		.pEngineName        = "No engine",
		.engineVersion      = VK_MAKE_VERSION(0, 0, 0),
		.apiVersion         = VK_API_VERSION_1_2
	};

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkInstanceCreateInfo createInfo
	{
		.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo        = &appInfo,
		.enabledLayerCount       = 0,
		.enabledExtensionCount   = glfwExtensionCount,
		.ppEnabledExtensionNames = glfwExtensions
	};

	if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Can't create instance");
	}

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	for(const auto& i: extensions)
	{
		std::cout << '\t' << i.extensionName << '\n';
	}
}
