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

#include "swapChainSupportDetails.hpp"

SwapChainSupportDetails::SwapChainSupportDetails(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface):
	capabilities(physicalDevice.getSurfaceCapabilitiesKHR(surface)),
	formats(physicalDevice.getSurfaceFormatsKHR(surface)),
	presentModes(physicalDevice.getSurfacePresentModesKHR(surface))
{}

vk::Extent2D SwapChainSupportDetails::getExtent(vk::Extent2D windowExtent) const
{
	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		return windowExtent;
	}
}

vk::SurfaceFormatKHR SwapChainSupportDetails::getSurfaceFormat() const
{
	using enum vk::Format;
	using enum vk::ColorSpaceKHR;

	for(const auto& availableFormat: formats)
	{
		if(availableFormat.format == eB8G8R8A8Srgb && availableFormat.colorSpace == eSrgbNonlinear)
			return availableFormat;
	}

	return formats[0];
}

vk::PresentModeKHR SwapChainSupportDetails::getPresentMode() const
{
	using enum vk::PresentModeKHR;

	for(const auto& availablePresentMode: presentModes)
	{
		if(availablePresentMode == eMailbox)
			return availablePresentMode;
	}

	return eFifo;
}
