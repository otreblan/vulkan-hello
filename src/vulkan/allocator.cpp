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

#include <utility>

#include "allocator.hpp"
#include "renderer.hpp"

Buffer::Buffer(Buffer&& other) noexcept:
	buffer(std::exchange(other.buffer,                 {})),
	allocation(std::exchange(other.allocation,         {})),
	allocationInfo(std::exchange(other.allocationInfo, {})),
	allocatorRef(std::exchange(other.allocatorRef,     {}))
{}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
	if(this != &other)
	{
		std::swap(buffer,         other.buffer);
		std::swap(allocation,     other.allocation);
		std::swap(allocationInfo, other.allocationInfo);
		std::swap(allocatorRef,   other.allocatorRef);
	}

	return *this;
}

Buffer::~Buffer()
{
	if(buffer)
	{
		vmaDestroyBuffer(allocatorRef, buffer, allocation);
	}
	buffer       = nullptr;
	allocation   = nullptr;
	allocatorRef = nullptr;
}

Buffer::operator vk::Buffer&()
{
	return buffer;
}

vk::Result Buffer::flush()
{
	return (vk::Result)vmaFlushAllocation(allocatorRef, allocation, 0, VK_WHOLE_SIZE);
}

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

Buffer Allocator::createBuffer(
	vk::DeviceSize size,
	vk::BufferUsageFlags usage,
	vk::MemoryPropertyFlags properties
)
{
	Buffer buffer;

	buffer.allocatorRef = allocator;

	vk::BufferCreateInfo bufferInfo({}, size, usage, vk::SharingMode::eExclusive);

	VkBufferCreateInfo _bufferInfo = bufferInfo;
	VkBuffer _buffer;

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
		VMA_ALLOCATION_CREATE_MAPPED_BIT;

	vmaCreateBuffer(allocator, &_bufferInfo, &allocCreateInfo, &_buffer, &buffer.allocation, &buffer.allocationInfo);

	buffer.buffer = _buffer;

	return buffer;
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
