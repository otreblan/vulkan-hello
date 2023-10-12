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

#include "config.hpp"
#include "helloTriangle.hpp"
#include "pipeline.hpp"
#include "uniformBufferObject.hpp"
#include "vertex.hpp"

Pipeline::Pipeline(HelloTriangle& parent):
	parent(parent)
{
}

void Pipeline::create()
{
	createSwapChain(*parent.physicalDevice);
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
}

void Pipeline::recreate()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(parent.window, &width, &height);

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(parent.window, &width, &height);
		glfwWaitEvents();
	}

	parent.device.waitIdle();

	swapChainFramebuffers.clear();

	commandBuffers.clear();

	graphicsPipeline.clear();
	pipelineLayout.clear();
	renderPass.clear();
	swapChainImageViews.clear();
	swapChain.clear();
	uniformBuffers.clear();
	uniformBuffersMemory.clear();
	descriptorPool.clear();

	create();
};

vk::SurfaceFormatKHR Pipeline::chooseSwapSurfaceFormat(const std::span<vk::SurfaceFormatKHR> availableFormats)
{
	using enum vk::Format;
	using enum vk::ColorSpaceKHR;

	for(const auto& availableFormat: availableFormats)
	{
		if(availableFormat.format == eB8G8R8A8Srgb && availableFormat.colorSpace == eSrgbNonlinear)
			return availableFormat;
	}

	return availableFormats[0];
}

vk::PresentModeKHR Pipeline::chooseSwapPresentMode(const std::span<vk::PresentModeKHR> availablePresentModes)
{
	using enum vk::PresentModeKHR;

	for(const auto& availablePresentMode: availablePresentModes)
	{
		if(availablePresentMode == eMailbox)
			return availablePresentMode;
	}

	return eFifo;
}

vk::Extent2D Pipeline::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetWindowSize(parent.window, &width, &height);

		return vk::Extent2D(width, height);
	}
}

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

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

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

	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass(
		{},
		vk::PipelineBindPoint::eGraphics,
		{},
		colorAttachmentRef
	);

	vk::SubpassDependency dependency(
		VK_SUBPASS_EXTERNAL,
		0,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlagBits::eNone,
		vk::AccessFlagBits::eColorAttachmentWrite
	);

	vk::RenderPassCreateInfo renderPassInfo({}, colorAttachment, subpass, dependency);

	renderPass = parent.device.createRenderPass(renderPassInfo);
}

void Pipeline::createFramebuffers()
{
	swapChainFramebuffers.reserve(swapChainImageViews.size());

	for(size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		vk::FramebufferCreateInfo framebufferInfo(
			{},
			*renderPass,
			*swapChainImageViews[i],
			swapChainExtent.width,
			swapChainExtent.height,
			1
		);

		swapChainFramebuffers.emplace_back(parent.device.createFramebuffer(framebufferInfo));
	}
}

void Pipeline::createUniformBuffers()
{
	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.reserve(swapChainImages.size());
	uniformBuffersMemory.reserve(swapChainImages.size());

	for(size_t i = 0; i < swapChainImages.size(); i++)
	{
		auto [buffer, bufferMemory] = parent.createBuffer(
			bufferSize,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);

		uniformBuffers.emplace_back(std::move(buffer));
		uniformBuffersMemory.emplace_back(std::move(bufferMemory));
	}
}

void Pipeline::createDescriptorPool()
{
	std::array<vk::DescriptorPoolSize, 2> poolSizes
	{
		vk::DescriptorPoolSize(
			vk::DescriptorType::eUniformBuffer,
			swapChainImages.size()
		),
		vk::DescriptorPoolSize(
			vk::DescriptorType::eCombinedImageSampler,
			swapChainImages.size()
		)
	};

	vk::DescriptorPoolCreateInfo poolInfo({}, swapChainImages.size(), poolSizes);

	descriptorPool = parent.device.createDescriptorPool(poolInfo);
}

void Pipeline::createDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(swapChainImages.size(), *parent.descriptorSetLayout);

	vk::DescriptorSetAllocateInfo allocInfo(*descriptorPool, layouts);

	descriptorSets = (*parent.device).allocateDescriptorSets(allocInfo);

	for(size_t i = 0; i < swapChainImages.size(); i++)
	{
		vk::DescriptorBufferInfo bufferInfo(*uniformBuffers[i], 0, sizeof(UniformBufferObject));

		vk::DescriptorImageInfo imageInfo(
			*parent.textureSampler,
			*parent.textureImageView,
			vk::ImageLayout::eShaderReadOnlyOptimal
		);

		std::array<vk::WriteDescriptorSet, 2> descriptorWrites
		{
			vk::WriteDescriptorSet(
				descriptorSets[i],
				0,
				0,
				vk::DescriptorType::eUniformBuffer,
				nullptr,
				bufferInfo,
				nullptr
			),
			vk::WriteDescriptorSet(
				descriptorSets[i],
				1,
				0,
				vk::DescriptorType::eCombinedImageSampler,
				imageInfo,
				nullptr,
				nullptr
			)
		};

		parent.device.updateDescriptorSets(descriptorWrites, nullptr);
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
		nullptr,
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

void Pipeline::createCommandBuffers()
{
	commandBuffers.reserve(swapChainFramebuffers.size());

	vk::CommandBufferAllocateInfo allocInfo(
		*parent.commandPool,
		vk::CommandBufferLevel::ePrimary,
		swapChainFramebuffers.size()
	);

	commandBuffers = parent.device.allocateCommandBuffers(allocInfo);

	for(size_t i = 0; i < commandBuffers.size(); i++)
	{
		vk::CommandBufferBeginInfo beginInfo({}, nullptr);

		commandBuffers[i].begin(beginInfo);

		vk::ClearValue clearColor({0,0,0,1});

		vk::RenderPassBeginInfo renderPassInfo(
			*renderPass,
			*swapChainFramebuffers[i],
			vk::Rect2D({0, 0}, swapChainExtent),
			clearColor
		);

		commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);

		vk::Buffer vertexBuffers[] = {*parent.vertexBuffer};
		vk::DeviceSize offsets[] = {0};

		commandBuffers[i].bindVertexBuffers(0, vertexBuffers, offsets);
		commandBuffers[i].bindIndexBuffer(*parent.indexBuffer, 0, vk::IndexType::eUint16);
		commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, descriptorSets[i], {});
		commandBuffers[i].drawIndexed(indices.size(), 1, 0, 0, 0);
		commandBuffers[i].endRenderPass();
		commandBuffers[i].end();
	}
}
