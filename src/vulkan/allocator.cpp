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

#include "allocator.hpp"
#include "renderer.hpp"

Allocator::Allocator(Renderer& root):
	root(root)
{
}

Allocator::~Allocator()
{
	vmaDestroyAllocator(allocator);
}

void Allocator::create()
{
	VmaVulkanFunctions vulkanFunctions
	{
		.vkGetInstanceProcAddr = &vkGetInstanceProcAddr,
		.vkGetDeviceProcAddr   = &vkGetDeviceProcAddr,
	};

	VmaAllocatorCreateInfo allocatorCreateInfo
	{
		.flags            = {},
		.physicalDevice   = *root.physicalDevice,
		.device           = *root.device,
		.pVulkanFunctions = &vulkanFunctions,
		.instance         = *root.instance,
	};

	vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> Allocator::createBuffer(
	vk::DeviceSize size,
	vk::BufferUsageFlags usage,
	vk::MemoryPropertyFlags properties
)
{
	vk::raii::Buffer buffer = nullptr;
	vk::raii::DeviceMemory bufferMemory = nullptr;

	vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);

	buffer = root.device.createBuffer(bufferInfo);

	vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

	vk::MemoryAllocateInfo allocInfo(
		memRequirements.size,
		findMemoryType(memRequirements.memoryTypeBits, properties)
	);

	bufferMemory = root.device.allocateMemory(allocInfo);

	buffer.bindMemory(*bufferMemory, 0);

	return {std::move(buffer), std::move(bufferMemory)};
}

uint32_t Allocator::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memProperties = root.physicalDevice.getMemoryProperties();

	for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}

	throw std::runtime_error("failed to find suitable memory type!");
}
