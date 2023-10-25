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

class Renderer;

class FrameData
{
private:
	// TODO: Make this a SOA
	static const int MAX_FRAMES_IN_FLIGHT = 2;

	struct Data
	{
		mutable vk::raii::Semaphore imageAvailable = nullptr;
		mutable vk::raii::Semaphore renderFinished = nullptr;
		mutable vk::raii::Fence     inFlight       = nullptr;

		vk::raii::CommandBuffer commandBuffer = nullptr;
	};

	Renderer& root;

	vk::raii::CommandPool commandPool = nullptr;

	std::array<Data, MAX_FRAMES_IN_FLIGHT> data;

	int currentFrame = 0;

	void createCommandPool();
	void createSyncObjects();
	void createCommandBuffers();

public:
	FrameData(Renderer& root);

	void create();

	int incrementFrame();
	int getCurrentFrame() const;

	vk::CommandPool   getCommandPool();
	vk::Semaphore     getImageAvailable();
	vk::Semaphore     getRenderFinished();
	vk::Fence         getInFlight();
	vk::CommandBuffer getCommandBuffer();

	friend class Renderer;
};
