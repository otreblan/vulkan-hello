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

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>

#include "mesh.hpp"
#include "vulkan/renderer.hpp"

Mesh::Mesh(const aiMesh& mesh)
{
	loadVertices(mesh);
	loadIndices(mesh);
}

void Mesh::loadVertices(const aiMesh& mesh)
{
	vertices.resize(mesh.mNumVertices);

	for(size_t i = 0; i < mesh.mNumVertices; i++)
	{
		const auto& v = mesh.mVertices[i];
		const auto& n = mesh.mNormals[i];

		vertices[i].pos = {v.x, v.y, v.z};

		if(mesh.HasVertexColors(0))
		{
			const auto& c = mesh.mColors[i][0];
			vertices[i].color = {c.r, c.g, c.b};
		}

		if(mesh.HasTextureCoords(0))
		{
			const auto& uv = mesh.mTextureCoords[0][i];
			vertices[i].texCoord = {uv.x, uv.y};
		}
		vertices[i].normal = {n.x, n.y, n.z};
	}
}

void Mesh::loadIndices(const aiMesh& mesh)
{
	indices.resize(mesh.mNumFaces*3);

	for(size_t i = 0; i < mesh.mNumFaces; i++)
	{
		const auto& face = mesh.mFaces[i];

		indices[3*i+0] = face.mIndices[0];
		indices[3*i+1] = face.mIndices[1];
		indices[3*i+2] = face.mIndices[2];
	}
}

void Mesh::clear()
{
	vertices.clear();
	indices.clear();
}

bool Mesh::uploadToGpu(Renderer& root) const
{
	auto [_vertexBuffer, _vertexBufferMemory] = uploadVertices(root);

	root.vertexBuffer       = std::move(_vertexBuffer);
	root.vertexBufferMemory = std::move(_vertexBufferMemory);

	auto [_indexBuffer, _indexBufferMemory] = uploadIndices(root);

	root.indexBuffer       = std::move(_indexBuffer);
	root.indexBufferMemory = std::move(_indexBufferMemory);

	return false;
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> Mesh::uploadVertices(Renderer& root) const
{
	using enum vk::BufferUsageFlagBits;
	using enum vk::MemoryPropertyFlagBits;

	vk::DeviceSize bufferSize = sizeof(decltype(vertices)::value_type) * vertices.size();

	auto [stagingBuffer, stagingBufferMemory] = root.createBuffer(
		bufferSize,
		eTransferSrc,
		eHostVisible | eHostCoherent
	);

	void* data = stagingBufferMemory.mapMemory(0, bufferSize);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	stagingBufferMemory.unmapMemory();

	auto [vertexBuffer, vertexBufferMemory] = root.createBuffer(
		bufferSize,
		eTransferDst | eVertexBuffer,
		eDeviceLocal
	);

	root.copyBuffer(*stagingBuffer, *vertexBuffer, bufferSize);

	return {std::move(vertexBuffer), std::move(vertexBufferMemory)};
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> Mesh::uploadIndices(Renderer& root) const
{
	using enum vk::BufferUsageFlagBits;
	using enum vk::MemoryPropertyFlagBits;

	vk::DeviceSize bufferSize = sizeof(decltype(indices)::value_type) * indices.size();

	auto [stagingBuffer, stagingBufferMemory] = root.createBuffer(
		bufferSize,
		eTransferSrc,
		eHostVisible | eHostCoherent
	);

	void* data = stagingBufferMemory.mapMemory(0, bufferSize);
	memcpy(data, indices.data(), (size_t) bufferSize);
	stagingBufferMemory.unmapMemory();

	auto [indexBuffer, indexBufferMemory] = root.createBuffer(
		bufferSize,
		eTransferDst | eIndexBuffer,
		eDeviceLocal
	);

	root.copyBuffer(*stagingBuffer, *indexBuffer, bufferSize);

	return {std::move(indexBuffer), std::move(indexBufferMemory)};
}

std::span<Vertex> Mesh::getVertices()
{
	return vertices;
}

std::span<const Vertex> Mesh::getVertices() const
{
	return vertices;
}

std::span<uint32_t> Mesh::getIndices()
{
	return indices;
}

std::span<const uint32_t> Mesh::getIndices() const
{
	return indices;
}
