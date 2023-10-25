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

#include <array>

#include <vulkan/vulkan_raii.hpp>

#include "allocator.hpp"
#include "uniformBufferObject.hpp"

class Renderer;

class FrameData
{
private:
	// TODO: Make this a SOA
	// TODO: Dynamic object buffer
	static const int MAX_FRAMES_IN_FLIGHT = 2;
	static const int MAX_OBJECTS          = 100000;

	struct Data
	{
		mutable vk::raii::Semaphore imageAvailable = nullptr;
		mutable vk::raii::Semaphore renderFinished = nullptr;
		mutable vk::raii::Fence     inFlight       = nullptr;

		vk::raii::CommandBuffer commandBuffer = nullptr;

		Buffer uniformBuffer;
		Buffer storageBuffer;

		vk::DescriptorSet descriptorSet;
	};

	Renderer& root;

	vk::raii::DescriptorPool descriptorPool = nullptr;

	std::array<Data, MAX_FRAMES_IN_FLIGHT> data;

	int currentFrame = 0;

	void createSyncObjects();
	void createCommandBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void createUniformBuffers();
	void createStorageBuffers();

public:
	FrameData(Renderer& root);

	void create();

	int incrementFrame();
	int getCurrentFrame() const;

	vk::Semaphore      getImageAvailable(size_t imageIndex);
	vk::Semaphore      getRenderFinished(size_t imageIndex);
	vk::Fence          getInFlight(size_t imageIndex);
	vk::CommandBuffer  getCommandBuffer(size_t imageIndex);
	vk::DescriptorSet  getDescriptorSet(size_t imageIndex);
	Buffer&            getUniformBuffer(size_t imageIndex);
	Buffer&            getStorageBuffer(size_t imageIndex);

	vk::Semaphore      getImageAvailable();
	vk::Semaphore      getRenderFinished();
	vk::Fence          getInFlight();
	vk::CommandBuffer  getCommandBuffer();
	vk::DescriptorPool getDescriptorPool();
	vk::DescriptorSet  getDescriptorSet();
	Buffer&            getUniformBuffer();
	Buffer&            getStorageBuffer();

	friend class Renderer;
};
