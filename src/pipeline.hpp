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
#include <filesystem>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "depth.hpp"
#include "swapChainSupportDetails.hpp"

class Renderer;

struct Pipeline
{
public:
	using path = std::filesystem::path;

	Renderer& parent;

	vk::raii::SwapchainKHR           swapChain = nullptr;
	std::vector<vk::Image>           swapChainImages;
	vk::Format                       swapChainImageFormat;
	vk::Extent2D                     swapChainExtent;
	std::vector<vk::raii::ImageView> swapChainImageViews;

	Depth depth;

	vk::raii::RenderPass                renderPass       = nullptr;
	vk::raii::PipelineLayout            pipelineLayout   = nullptr;
	vk::raii::Pipeline                  graphicsPipeline = nullptr;
	std::vector<vk::raii::Framebuffer>  swapChainFramebuffers;
	std::vector<vk::raii::Buffer>       uniformBuffers;
	std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;

	std::vector<vk::raii::CommandBuffer> commandBuffers;

	vk::raii::DescriptorPool       descriptorPool = nullptr;
	std::vector<vk::DescriptorSet> descriptorSets;

	Pipeline(Renderer& parent);
	void create();
	void recreate();

	void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

private:
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::span<vk::SurfaceFormatKHR> availableFormats);
	vk::PresentModeKHR chooseSwapPresentMode(const std::span<vk::PresentModeKHR> availablePresentModes);
	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
	void createImageViews();
	void createSwapChain(vk::PhysicalDevice physicalDevice);

	static std::vector<char> readFile(const path& filepath);
	vk::raii::ShaderModule createShaderModule(std::span<char> code);
	void createRenderPass();
	void createFramebuffers();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void createGraphicsPipeline();
	void createCommandBuffers();
};
