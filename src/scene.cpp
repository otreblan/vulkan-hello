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

#include "component/meshInstance.hpp"
#include "component/properties.hpp"
#include "component/transform.hpp"
#include "scene.hpp"
#include "utils.hpp"

Scene::Scene(const std::filesystem::path& scenePath)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		scenePath,
		aiProcess_ImproveCacheLocality |
		aiProcess_JoinIdenticalVertices |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_Triangulate
	);

	if(scene == nullptr)
		throw importer.GetErrorString();

	name = scene->mName.C_Str();

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

entt::entity Scene::loadHierarchy(aiNode* node, entt::entity parent)
{
	if(node == nullptr)
		return entt::null;

	auto entity = registry.create();

	registry.emplace<component::Transform>(entity, toGlm(node->mTransformation));
	registry.emplace<component::Properties>(entity, node->mName.C_Str());

	component::MeshInstance instance;

	for(size_t i = 0; i < node->mNumMeshes; i++)
	{
		instance.meshes.emplace_back(node->mMeshes[i]);
	}

	registry.emplace<component::MeshInstance>(entity, std::move(instance));

	component::Transform::Relationship relationship
	{
		.children = {},
		.parent   = parent
	};

	relationship.children.reserve(node->mNumChildren);

	for(size_t i = 0; i < node->mNumChildren; i++)
	{
		if(auto child = loadHierarchy(node->mChildren[i], entity); child != entt::null)
		{
			relationship.children.emplace_back(child);
		}
	}

	registry.emplace<component::Transform::Relationship>(entity, std::move(relationship));

	return entity;
}
