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

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "component/collider.hpp"
#include "component/meshInstance.hpp"
#include "component/properties.hpp"
#include "component/transform.hpp"
#include "scene.hpp"
#include "utils.hpp"

Scene::Scene(const std::filesystem::path& scenePath)
{
	using namespace ecs::component;

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		scenePath,
		aiProcess_GenBoundingBoxes |
		aiProcess_ImproveCacheLocality |
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate
	);

	if(scene == nullptr)
		throw importer.GetErrorString();

	name = scene->mName.C_Str();

	entt::sigh_helper{registry}
		.with<Transform>()
			.on_construct<&entt::registry::emplace<Transform::Relationship>>()
		.with<Transform::Relationship>()
			.on_destroy<&Scene::updateHierarchy>()
	;

	loadMeshes({scene->mMeshes, scene->mNumMeshes});
	loadHierarchy(scene->mRootNode, entt::null);
}

void Scene::loadMeshes(const std::span<aiMesh*> newMeshes)
{
	meshes.reserve(newMeshes.size());

	for(const auto* mesh: newMeshes)
	{
		if(mesh)
		{
			meshes.emplace_back(*mesh);
		}
	}
}

entt::entity Scene::loadHierarchy(const aiNode* node, entt::entity parent)
{
	using namespace ecs::component;

	if(node == nullptr)
		return entt::null;

	auto entity = registry.create();

	if(parent == entt::null)
		root = entity;

	registry.emplace<Transform>(entity, toGlm(node->mTransformation));

	const auto& name = registry.emplace<Properties>(entity, node->mName.C_Str()).name;

	if(node->mNumMeshes > 0)
	{
		std::span indexSpan(node->mMeshes, node->mMeshes+node->mNumMeshes);

		registry.emplace<MeshInstance>(entity, MeshInstance({indexSpan.begin(), indexSpan.end()}));

		glm::vec3 aabbMin(std::numeric_limits<glm::vec3::value_type>::max());
		glm::vec3 aabbMax(std::numeric_limits<glm::vec3::value_type>::min());

		for(auto i: indexSpan)
		{
			aabbMin = glm::min(aabbMin, meshes[i].aabbMin);
			aabbMax = glm::max(aabbMax, meshes[i].aabbMax);
		}

		registry.emplace<Collider>(entity, Collider(aabbMin, aabbMax, name == "Plane" ? 0 : 1));
	}

	entt::entity children[node->mNumChildren];

	for(size_t i = 0; i < node->mNumChildren; i++)
	{
		children[i] = loadHierarchy(node->mChildren[i], entity);
	}

	auto& tRelationship = registry.get<Transform::Relationship>(entity);

	tRelationship.parent = parent;
	tRelationship.children.insert(children, children+node->mNumChildren);

	return entity;
}

std::vector<Renderable> Scene::getRenderables() const
{
	using namespace ecs::component;

	std::vector<Renderable> renderables;
	auto view = registry.view<MeshInstance>();

	for(entt::entity entity: view)
	{
		glm::mat4 matrix(1);

		for(auto e = entity; e != entt::null; e = pGroup.get<Transform::Relationship>(e).parent)
		{
			matrix = pGroup.get<Transform>(e).matrix * matrix;
		}

		for(auto i: view.get<MeshInstance>(entity).meshes)
		{
			renderables.emplace_back(
				matrix,
				meshes[i].getVertexBuffer(),
				meshes[i].getIndexBuffer(),
				meshes[i].getIndices().size()
			);
		}
	}

	return renderables;
}

void Scene::uploadToGpu(Renderer& renderer)
{
	for(auto& mesh: meshes)
	{
		mesh.uploadToGpu(renderer);
	}
}

void Scene::updateHierarchy(entt::registry& registry, entt::entity entity)
{
	using namespace ecs::component;

	auto trView = registry.view<Transform::Relationship>();

	const auto&& [self] = trView.get(entity);

	if(self.parent != entt::null)
	{
		auto&& [parent] = trView.get(self.parent);

		parent.children.erase(entity);
		parent.children.insert(self.children.begin(), self.children.end());
	}

	for(entt::entity e: self.children)
	{
		auto&& [children] = trView.get(e);

		children.parent = self.parent;
	}
}
