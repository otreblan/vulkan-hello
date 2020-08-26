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

#include <vertex.hpp>

const std::array<VkVertexInputAttributeDescription, 3> Vertex::attributeDescriptions
{
	VkVertexInputAttributeDescription
	{
		.location = 0,
		.binding  = 0,
		.format   = VK_FORMAT_R32G32_SFLOAT,
		.offset   = offsetof(Vertex, pos),
	},
	VkVertexInputAttributeDescription
	{
		.location = 1,
		.binding  = 0,
		.format   = VK_FORMAT_R32G32B32_SFLOAT,
		.offset   = offsetof(Vertex, color),
	},
	VkVertexInputAttributeDescription
	{
		.location = 2,
		.binding  = 0,
		.format   = VK_FORMAT_R32G32_SFLOAT,
		.offset   = offsetof(Vertex, texCoord),
	}
};

const VkVertexInputBindingDescription Vertex::bindingDescription
{
	.binding   = 0,
	.stride    = sizeof(Vertex),
	.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
};

const std::vector<Vertex> vertices =
{
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
