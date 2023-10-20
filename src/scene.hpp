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

#include <entt/entt.hpp>

#include "component/transform.hpp"
#include "mesh.hpp"

struct aiMesh;
struct aiNode;

struct Renderable
{
	const glm::mat4 transform;
	const Mesh&     mesh;
	// Material& material;
};

struct Scene
{
	Scene(const std::filesystem::path& scenePath);

	std::string name;
	entt::entity root = entt::null;

	entt::registry    registry;
	std::vector<Mesh> meshes;

	std::vector<Renderable> getRenderables();

private:
	void loadMeshes(const std::span<aiMesh*> newMeshes);
	entt::entity loadHierarchy(aiNode* node, entt::entity parent);
};
