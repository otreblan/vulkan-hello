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
#include "queueFamilyIndices.hpp"
#include "renderer.hpp"

FrameData::FrameData(Renderer& root):
	root(root)
{}

void FrameData::create()
{
	createCommandPool();
	createSyncObjects();
	createCommandBuffers();
}

int FrameData::incrementFrame()
{
	return (currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT);
}

int FrameData::getCurrentFrame() const
{
	return currentFrame;
}

vk::CommandPool FrameData::getCommandPool()
{
	return *commandPool;
}

vk::Semaphore FrameData::getImageAvailable()
{
	return *data[getCurrentFrame()].imageAvailable;
}

vk::Semaphore FrameData::getRenderFinished()
{
	return *data[getCurrentFrame()].renderFinished;
}

vk::Fence FrameData::getInFlight()
{
	return *data[getCurrentFrame()].inFlight;
}

vk::CommandBuffer FrameData::getCommandBuffer()
{
	return *data[getCurrentFrame()].commandBuffer;
}

void FrameData::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = root.findQueueFamilies(*root.physicalDevice);

	vk::CommandPoolCreateInfo poolInfo(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		queueFamilyIndices.graphicsFamily.value()
	);

	commandPool = root.device.createCommandPool(poolInfo);
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
		*commandPool,
		vk::CommandBufferLevel::ePrimary,
		data.size()
	);

	auto buffers = root.device.allocateCommandBuffers(allocInfo);

	for(size_t i = 0; i < data.size(); i++)
	{
		data[i].commandBuffer = std::move(buffers[i]);
	}
}
