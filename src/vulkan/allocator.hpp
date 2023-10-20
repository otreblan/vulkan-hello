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

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>

class Renderer;

class Allocator
{
public:
	Allocator(Renderer& root);
	~Allocator();

	void create();

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties
	);

private:
	VmaAllocator allocator;
	Renderer&    root;
};
