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

#include "frameData.hpp"
#include "renderer.hpp"
#include "shaderStorageBufferObject.hpp"

FrameData::FrameData(Renderer& root):
	root(root)
{}

void FrameData::create()
{
	createSyncObjects();
	createCommandBuffers();
	createDescriptorPool();
	createUniformBuffers();
	createStorageBuffers();
	createDescriptorSets();
}

int FrameData::incrementFrame()
{
	return (currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT);
}

int FrameData::getCurrentFrame() const
{
	return currentFrame;
}

vk::Semaphore FrameData::getImageAvailable(size_t imageIndex)
{
	return *data[imageIndex].imageAvailable;
}

vk::Semaphore FrameData::getRenderFinished(size_t imageIndex)
{
	return *data[imageIndex].renderFinished;
}

vk::Fence FrameData::getInFlight(size_t imageIndex)
{
	return *data[imageIndex].inFlight;
}

vk::CommandBuffer FrameData::getCommandBuffer(size_t imageIndex)
{
	return *data[imageIndex].commandBuffer;
}

vk::DescriptorSet FrameData::getDescriptorSet(size_t imageIndex)
{
	return data[imageIndex].descriptorSet;
}

Buffer& FrameData::getUniformBuffer(size_t imageIndex)
{
	return data[imageIndex].uniformBuffer;
}

Buffer& FrameData::getStorageBuffer(size_t imageIndex)
{
	return data[imageIndex].storageBuffer;
}

vk::Semaphore FrameData::getImageAvailable()
{
	return getImageAvailable(getCurrentFrame());
}

vk::Semaphore FrameData::getRenderFinished()
{
	return getRenderFinished(getCurrentFrame());
}

vk::Fence FrameData::getInFlight()
{
	return getInFlight(getCurrentFrame());
}

vk::CommandBuffer FrameData::getCommandBuffer()
{
	return getCommandBuffer(getCurrentFrame());
}

vk::DescriptorPool FrameData::getDescriptorPool()
{
	return *descriptorPool;
}

vk::DescriptorSet FrameData::getDescriptorSet()
{
	return getDescriptorSet(getCurrentFrame());
}

Buffer& FrameData::getUniformBuffer()
{
	return getUniformBuffer(getCurrentFrame());
}

Buffer& FrameData::getStorageBuffer()
{
	return getStorageBuffer(getCurrentFrame());
}

void FrameData::createSyncObjects()
{
	vk::SemaphoreCreateInfo semaphoreInfo;

	// Start signaled to avoid a deadlock in the first frame.
	vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

	for(size_t i = 0; i < data.size(); i++)
	{
		data[i].imageAvailable = root.device.createSemaphore(semaphoreInfo);
		data[i].renderFinished = root.device.createSemaphore(semaphoreInfo);
		data[i].inFlight       = root.device.createFence(fenceInfo);
	}
}
void FrameData::createCommandBuffers()
{
	vk::CommandBufferAllocateInfo allocInfo(
		*root.commandPool,
		vk::CommandBufferLevel::ePrimary,
		data.size()
	);

	auto buffers = root.device.allocateCommandBuffers(allocInfo);

	for(size_t i = 0; i < data.size(); i++)
	{
		data[i].commandBuffer = std::move(buffers[i]);
	}
}

void FrameData::createDescriptorPool()
{
	vk::DescriptorPoolSize poolSizes[] =
	{
		vk::DescriptorPoolSize(
			vk::DescriptorType::eUniformBuffer,
			MAX_FRAMES_IN_FLIGHT
		),
		vk::DescriptorPoolSize(
			vk::DescriptorType::eStorageBuffer,
			MAX_FRAMES_IN_FLIGHT
		),
		vk::DescriptorPoolSize(
			vk::DescriptorType::eCombinedImageSampler,
			MAX_FRAMES_IN_FLIGHT
		)
	};

	vk::DescriptorPoolCreateInfo poolInfo({}, MAX_FRAMES_IN_FLIGHT, poolSizes);

	descriptorPool = root.device.createDescriptorPool(poolInfo);
}

void FrameData::createDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *root.descriptorSetLayout);

	vk::DescriptorSetAllocateInfo allocInfo(*descriptorPool, layouts);

	auto descriptorSets = (*root.device).allocateDescriptorSets(allocInfo);

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		data[i].descriptorSet = descriptorSets[i];
	}

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vk::DescriptorBufferInfo bufferInfo(
			data[i].uniformBuffer,
			0,
			sizeof(UniformBufferObject)
		);

		vk::DescriptorBufferInfo storageBufferInfo(
			data[i].storageBuffer,
			0,
			sizeof(ShaderStorageBufferObject)*MAX_OBJECTS
		);

		vk::DescriptorImageInfo imageInfo(
			*root.textureSampler,
			*root.textureImageView,
			vk::ImageLayout::eShaderReadOnlyOptimal
		);

		vk::WriteDescriptorSet descriptorWrites[] =
		{
			vk::WriteDescriptorSet(
				data[i].descriptorSet,
				0,
				0,
				vk::DescriptorType::eUniformBuffer,
				nullptr,
				bufferInfo,
				nullptr
			),
			vk::WriteDescriptorSet(
				data[i].descriptorSet,
				1,
				0,
				vk::DescriptorType::eStorageBuffer,
				nullptr,
				storageBufferInfo,
				nullptr
			),
			vk::WriteDescriptorSet(
				data[i].descriptorSet,
				2,
				0,
				vk::DescriptorType::eCombinedImageSampler,
				imageInfo,
				nullptr,
				nullptr
			)
		};

		root.device.updateDescriptorSets(descriptorWrites, nullptr);
	}
}

void FrameData::createUniformBuffers()
{
	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	for(size_t i = 0; i < data.size(); i++)
	{
		data[i].uniformBuffer = root.allocator.createBuffer(
			bufferSize,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
	}
}

void FrameData::createStorageBuffers()
{
	vk::DeviceSize bufferSize = sizeof(ShaderStorageBufferObject)*MAX_OBJECTS;

	for(size_t i = 0; i < data.size(); i++)
	{
		data[i].storageBuffer = root.allocator.createBuffer(
			bufferSize,
			vk::BufferUsageFlagBits::eStorageBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
	}
}
