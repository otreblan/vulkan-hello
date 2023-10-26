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
		aiProcess_GenBoundingBoxes |
		aiProcess_ImproveCacheLocality |
		aiProcess_JoinIdenticalVertices |
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

entt::entity Scene::loadHierarchy(const aiNode* node, entt::entity parent)
{
	using namespace ecs::component;

	if(node == nullptr)
		return entt::null;

	auto entity = registry.create();

	if(parent == entt::null)
		root = entity;

	registry.emplace<Transform>(entity, toGlm(node->mTransformation));
	registry.emplace<Properties>(entity, node->mName.C_Str());
	registry.emplace<Transform::Parent>(entity, parent);

	if(node->mNumMeshes > 0)
		registry.emplace<MeshInstance>(entity, MeshInstance({node->mMeshes, node->mMeshes+node->mNumMeshes}));

	entt::entity children[node->mNumChildren];

	for(size_t i = 0; i < node->mNumChildren; i++)
	{
		children[i] = loadHierarchy(node->mChildren[i], entity);
	}

	registry.emplace<Transform::Children>(entity, Transform::Children({children, children+node->mNumChildren}));

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

		for(auto e = entity; e != entt::null; e = pGroup.get<Transform::Parent>(e).entity)
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
