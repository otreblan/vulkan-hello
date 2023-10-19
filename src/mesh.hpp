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

#include <filesystem>
#include <span>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

#include "vertex.hpp"

#pragma once

struct aiMesh;
class Renderer;

// TODO: Make this data-oriented
class Mesh
{
private:
	std::vector<Vertex>   vertices;
	std::vector<uint32_t> indices;

	void loadVertices(const aiMesh& mesh);
	void loadIndices(const aiMesh& mesh);

	// TODO: Use vulkan memory allocator
	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> uploadVertices(Renderer& root);
	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> uploadIndices(Renderer& root);

public:
	Mesh(const aiMesh& mesh);

	bool load();
	void clear();

	bool uploadToGpu(Renderer& root);

	std::span<Vertex>       getVertices();
	std::span<const Vertex> getVertices() const;

	std::span<uint32_t>       getIndices();
	std::span<const uint32_t> getIndices() const;
};
