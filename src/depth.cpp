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

#include "depth.hpp"
#include "helloTriangle.hpp"

vk::Format Depth::findSupportedFormat(const std::span<const vk::Format> candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	for(const auto& format: candidates)
	{
		vk::FormatProperties props = root.physicalDevice.getFormatProperties(format);

		switch(tiling)
		{
			case vk::ImageTiling::eLinear:
				if((props.linearTilingFeatures & features) == features)
					return format;
				break;

			case vk::ImageTiling::eOptimal:
				if((props.optimalTilingFeatures & features) == features)
					return format;
				break;

			default:
				break;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

bool Depth::hasStencilComponent(vk::Format format)
{
	switch(format)
	{
		case vk::Format::eD32SfloatS8Uint:
		case vk::Format::eD24UnormS8Uint:
			return true;

		default:
			return false;
	}
}

Depth::Depth(HelloTriangle& root):
	root(root)
{
}

void Depth::create()
{
	vk::Format depthFormat = getFormat();

	auto [_image, _imageMemory] = root.createImage(
		root.pipeline.swapChainExtent.width,
		root.pipeline.swapChainExtent.height,
		depthFormat,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);

	image       = std::move(_image);
	imageMemory = std::move(_imageMemory);
	imageView   = root.createImageView(*image, depthFormat, vk::ImageAspectFlagBits::eDepth);

	//root.transitionImageLayout(*image, depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

void Depth::clear()
{
	image.clear();
	imageMemory.clear();
	imageView.clear();
}

vk::Format Depth::getFormat()
{
	vk::Format formats[] = {
		vk::Format::eD32Sfloat,
		vk::Format::eD32SfloatS8Uint,
		vk::Format::eD24UnormS8Uint
	};

	return findSupportedFormat(
		formats,
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
}

vk::Image Depth::getImage()
{
	return *image;
}

vk::DeviceMemory Depth::getImageMemory()
{
	return *imageMemory;
}

vk::ImageView Depth::getImageView()
{
	return *imageView;
}
