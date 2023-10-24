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

#include <filesystem>
#include <span>
#include <type_traits>

#include <entt/entt.hpp>

#include "component/transform.hpp"
#include "mesh.hpp"
#include "utils.hpp"

class Renderer;
struct aiMesh;
struct aiNode;

struct Renderable
{
	glm::mat4  transform;
	vk::Buffer vertexBuffer;
	vk::Buffer indexBuffer;
	uint32_t   indexCount;
	// Material& material;
};

struct Scene
{
	using Transform = component::Transform;
	using pgroup_t  = group_t<const Transform, const Transform::Parent>;

	Scene(const std::filesystem::path& scenePath);

	std::string name;
	entt::entity root = entt::null;

	entt::registry    registry;
	std::vector<Mesh> meshes;

	const pgroup_t pGroup = registry.group<const Transform, const Transform::Parent>();

	std::vector<Renderable> getRenderables() const;
	void uploadToGpu(Renderer& renderer);

private:
	void loadMeshes(const std::span<aiMesh*> newMeshes);
	entt::entity loadHierarchy(const aiNode* node, entt::entity parent);
};
