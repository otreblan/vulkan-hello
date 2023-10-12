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

#include "singleCommand.hpp"

SingleCommand::SingleCommand(vk::raii::Device& device, vk::CommandPool commandPool, vk::Queue queue):
	queue(queue)
{
	vk::CommandBufferAllocateInfo allocInfo(
		commandPool,
		vk::CommandBufferLevel::ePrimary,
		1
	);

	buffer = std::move(device.allocateCommandBuffers(allocInfo).front());

	vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	buffer.begin(beginInfo);
}

vk::CommandBuffer const & SingleCommand::getBuffer()
{
	return *buffer;
}

void SingleCommand::submit()
{
	if(!*buffer)
		return;

	buffer.end();

	vk::SubmitInfo submitInfo(
		{},
		{},
		*buffer
	);

	queue.submit(submitInfo);
	queue.waitIdle();
}

SingleCommand::~SingleCommand()
{
	submit();
}
