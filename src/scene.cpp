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

#include "scene.hpp"

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

	loadMeshes({scene->mMeshes, scene->mNumMeshes});

	auto* root = scene->mRootNode;
	// TODO: Load scene
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
