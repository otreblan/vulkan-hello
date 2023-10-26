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

#include <fstream>

#include "../config.hpp"
#include "../engine.hpp"
#include "../scene.hpp"
#include "../vertex.hpp"
#include "pipeline.hpp"
#include "renderer.hpp"
#include "uniformBufferObject.hpp"

Pipeline::Pipeline(Renderer& parent):
	parent(parent),
	depth(parent)
{
}

void Pipeline::create()
{
	createSwapChain(*parent.physicalDevice);
	createImageViews();
	depth.create();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
}

void Pipeline::recreate()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(parent.engine.getWindow(), &width, &height);

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(parent.engine.getWindow(), &width, &height);
		glfwWaitEvents();
	}

	parent.device.waitIdle();

	depth.clear();
	swapChainFramebuffers.clear();

	graphicsPipeline.clear();
	pipelineLayout.clear();
	renderPass.clear();
	swapChainImageViews.clear();
	swapChain.clear();

	create();
};

void Pipeline::createImageViews()
{
	swapChainImageViews.reserve(swapChainImages.size());
	for(const auto& image: swapChainImages)
	{
		swapChainImageViews.emplace_back(parent.createImageView(image, swapChainImageFormat));
	}
}

void Pipeline::createSwapChain(vk::PhysicalDevice physicalDevice)
{
	SwapChainSupportDetails swapChainSupport = parent.querySwapChainSupport(physicalDevice);

	vk::SurfaceFormatKHR surfaceFormat = swapChainSupport.getSurfaceFormat();
	vk::PresentModeKHR   presentMode   = swapChainSupport.getPresentMode();
	vk::Extent2D         extent        = swapChainSupport.getExtent(parent.getWindowSize());

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if(swapChainSupport.capabilities.maxImageCount != 0)
		imageCount = std::min(imageCount, swapChainSupport.capabilities.maxImageCount);

	vk::SwapchainCreateInfoKHR createInfo(
		{},
		*parent.surface,
		imageCount,
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		{},
		{},
		swapChainSupport.capabilities.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		presentMode,
		true,
		nullptr
	);

	QueueFamilyIndices indices = parent.findQueueFamilies(*parent.physicalDevice);

	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if(indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices   = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode      = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices   = nullptr; // Optional
	}

	swapChain            = parent.device.createSwapchainKHR(createInfo);
	swapChainImages      = swapChain.getImages();
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent      = extent;
}

std::vector<char> Pipeline::readFile(const path& filepath)
{
	uintmax_t size = std::filesystem::file_size(filepath);

	std::vector<char> buffer(size);
	std::ifstream file(filepath, std::ios::binary);

	if(!file.is_open())
		throw std::runtime_error("failed to open file!");

	file.read(buffer.data(), size);
	file.close();

	return buffer;
}

vk::raii::ShaderModule Pipeline::createShaderModule(std::span<char> code)
{
	vk::ShaderModuleCreateInfo createInfo({}, code.size(), (const uint32_t*)code.data());

	return parent.device.createShaderModule(createInfo);
}

void Pipeline::createRenderPass()
{
	vk::AttachmentDescription colorAttachment(
		{},
		swapChainImageFormat,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::ePresentSrcKHR
	);

	vk::AttachmentDescription depthAttachment(
		{},
		depth.getFormat(),
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eDontCare,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);

	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass(
		{},
		vk::PipelineBindPoint::eGraphics,
		{},
		colorAttachmentRef,
		{},
		&depthAttachmentRef
	);

	vk::SubpassDependency dependency(
		VK_SUBPASS_EXTERNAL,
		0,
		vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests,
		vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
		vk::AccessFlagBits::eDepthStencilAttachmentWrite,
		vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
	);

	vk::AttachmentDescription attachments[] = {colorAttachment, depthAttachment};

	vk::RenderPassCreateInfo renderPassInfo({}, attachments, subpass, dependency);

	renderPass = parent.device.createRenderPass(renderPassInfo);
}

void Pipeline::createFramebuffers()
{
	swapChainFramebuffers.reserve(swapChainImageViews.size());

	for(size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		vk::ImageView attachments[] = {*swapChainImageViews[i], depth.getImageView()};

		vk::FramebufferCreateInfo framebufferInfo(
			{},
			*renderPass,
			attachments,
			swapChainExtent.width,
			swapChainExtent.height,
			1
		);

		swapChainFramebuffers.emplace_back(parent.device.createFramebuffer(framebufferInfo));
	}
}

void Pipeline::createGraphicsPipeline()
{

	auto vertShaderCode = readFile(shadersDir/"vert.spv");
	auto fragShaderCode = readFile(shadersDir/"frag.spv");

	auto vertShaderModule = createShaderModule(vertShaderCode);
	auto fragShaderModule = createShaderModule(fragShaderCode);

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
		{},
		vk::ShaderStageFlagBits::eVertex,
		*vertShaderModule,
		"main"
	);

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
		{},
		vk::ShaderStageFlagBits::eFragment,
		*fragShaderModule,
		"main"
	);

	vk::PipelineShaderStageCreateInfo shaderStages[] =
	{
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
		{},
		Vertex::bindingDescription,
		Vertex::attributeDescriptions
	);

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
		{},
		vk::PrimitiveTopology::eTriangleList,
		false
	);

	vk::Viewport viewport(
		0,
		0,
		swapChainExtent.width,
		swapChainExtent.height,
		0,
		1
	);

	vk::Rect2D scissor(vk::Offset2D(0, 0), swapChainExtent);

	vk::PipelineViewportStateCreateInfo viewportState({}, viewport, scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizer(
		{},
		false,
		false,
		vk::PolygonMode::eFill,
		vk::CullModeFlagBits::eBack,
		// OpenGL -> Vulkan
		vk::FrontFace::eCounterClockwise,
		false,
		0,
		0,
		0,
		1
	);

	vk::PipelineMultisampleStateCreateInfo multisampling(
		{},
		vk::SampleCountFlagBits::e1,
		false,
		1,
		nullptr,
		false,
		false
	);

	vk::PipelineDepthStencilStateCreateInfo depthStencil(
		{},
		true,
		true,
		vk::CompareOp::eLess,
		false,
		false
	);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment(
		false,
		vk::BlendFactor::eOne,
		vk::BlendFactor::eZero,
		vk::BlendOp::eAdd,
		vk::BlendFactor::eOne,
		vk::BlendFactor::eZero,
		vk::BlendOp::eAdd,
		vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA
	);

	vk::PipelineColorBlendStateCreateInfo colorBlending(
		{},
		false,
		vk::LogicOp::eCopy,
		colorBlendAttachment,
		{0.0f, 0.0f, 0.0f, 0.0f}
	);

	vk::DynamicState dynamicStates[] = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eLineWidth
	};

	[[maybe_unused]]
	vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *parent.descriptorSetLayout);

	pipelineLayout = parent.device.createPipelineLayout(pipelineLayoutInfo);

	vk::GraphicsPipelineCreateInfo pipelineInfo(
		{},
		shaderStages,
		&vertexInputInfo,
		&inputAssembly,
		nullptr,
		&viewportState,
		&rasterizer,
		&multisampling,
		&depthStencil,
		&colorBlending,
		nullptr,
		*pipelineLayout,
		*renderPass,
		0,
		nullptr,
		-1
	);

	graphicsPipeline = parent.device.createGraphicsPipeline(nullptr, pipelineInfo);
}

void Pipeline::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
	vk::CommandBufferBeginInfo beginInfo({}, nullptr);

	commandBuffer.begin(beginInfo);

	vk::ClearValue clearValues[] = {vk::ClearColorValue(0, 0, 0, 1), vk::ClearDepthStencilValue(1, 0)};

	vk::RenderPassBeginInfo renderPassInfo(
		*renderPass,
		*swapChainFramebuffers[imageIndex],
		vk::Rect2D({0, 0}, swapChainExtent),
		clearValues
	);

	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);

	//for(const auto& r: parent.renderables)
	for(size_t i = 0; i < parent.renderables.size(); i++)
	{
		vk::Buffer     vertexBuffers[] = {parent.renderables[i].vertexBuffer};
		vk::DeviceSize offsets[]       = {0};

		commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
		commandBuffer.bindIndexBuffer(parent.renderables[i].indexBuffer, 0, vk::IndexType::eUint32);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, parent.frameData.getDescriptorSet(), {});
		commandBuffer.drawIndexed(parent.renderables[i].indexCount, 1, 0, 0, i);
	}
	commandBuffer.endRenderPass();
	commandBuffer.end();
}
