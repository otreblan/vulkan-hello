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

#include <span>

#include <vulkan/vulkan_raii.hpp>

class Renderer;

class Depth
{
private:
	Renderer& root;

	vk::raii::Image        image       = nullptr;
	vk::raii::DeviceMemory imageMemory = nullptr;
	vk::raii::ImageView    imageView   = nullptr;

	vk::Format findSupportedFormat(const std::span<const vk::Format> candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	bool hasStencilComponent(vk::Format format);

public:
	Depth(Renderer& root);

	void create();
	void clear();

	vk::Format       getFormat();
	vk::Image        getImage();
	vk::DeviceMemory getImageMemory();
	vk::ImageView    getImageView();
};
