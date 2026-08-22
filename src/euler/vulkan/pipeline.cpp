/* SPDX-License-Identifier: ISC */

#include <numeric>

#include "euler/vulkan/pipeline.h"
#include "euler/vulkan/renderer.h"
#include "euler/vulkan/window.h"

euler::vulkan::Pipeline::Pipeline(Window &window,
    const std::vector<util::Reference<Shader>> &shaders)
    : _window(window)
    , _shaders(shaders)
    , _layout(create_layout())
    , _pipeline(create_pipeline())
{
}

vk::raii::PipelineLayout
euler::vulkan::Pipeline::create_layout()
{
	static constexpr vk::PipelineLayoutCreateInfo layout_info {};
	return vk::raii::PipelineLayout(renderer()->device(), layout_info);
}

vk::raii::Pipeline
euler::vulkan::Pipeline::create_pipeline()
{
	static constexpr vk::PipelineVertexInputStateCreateInfo
	    vertex_input_info {
		    /* TODO */
	    };
	static constexpr vk::PipelineInputAssemblyStateCreateInfo
	    input_assembly_info {
		    .topology = vk::PrimitiveTopology::eTriangleList,
	    };
	const auto [width, height] = window().surface().extent();
	const vk::Viewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(width),
		.height = static_cast<float>(height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	const vk::Rect2D scissor = {
		.offset = { 0, 0 },
		.extent = { width, height },
	};

	const vk::PipelineViewportStateCreateInfo viewport_state_info {
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};
	static constexpr vk::PipelineRasterizationStateCreateInfo
	    rasterizer_info {
		    .polygonMode = vk::PolygonMode::eFill,
		    .cullMode = vk::CullModeFlagBits::eBack,
		    .frontFace = vk::FrontFace::eClockwise,
		    .lineWidth = 1.0f,
	    };
	static constexpr vk::PipelineMultisampleStateCreateInfo
	    multisampling_info {
		    .rasterizationSamples = vk::SampleCountFlagBits::e1,
	    };
	static constexpr vk::PipelineColorBlendAttachmentState
	    color_blend_attachment {
		    .blendEnable = vk::True,
		    .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
		    .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
		    .colorBlendOp = vk::BlendOp::eAdd,
		    .srcAlphaBlendFactor = vk::BlendFactor::eOne,
		    .dstAlphaBlendFactor = vk::BlendFactor::eZero,
		    .alphaBlendOp = vk::BlendOp::eAdd,
		    .colorWriteMask = vk::ColorComponentFlagBits::eR
		        | vk::ColorComponentFlagBits::eG
		        | vk::ColorComponentFlagBits::eB
		        | vk::ColorComponentFlagBits::eA,
	    };
	static constexpr vk::PipelineColorBlendStateCreateInfo
	    color_blending_info {
		    .attachmentCount = 1,
		    .pAttachments = &color_blend_attachment,
	    };
	static constexpr std::array dynamic_states = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
	};
	static constexpr vk::PipelineDynamicStateCreateInfo dynamic_state_info {
		.dynamicStateCount
		= static_cast<uint32_t>(dynamic_states.size()),
		.pDynamicStates = dynamic_states.data(),
	};
	std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
	const auto total_count = std::accumulate(_shaders.begin(),
	    _shaders.end(), static_cast<size_t>(0),
	    [](const size_t acc, const util::Reference<Shader> &shader) {
		    return acc + shader->stage_count();
	    });
	shader_stages.reserve(total_count);
	for (const auto &shader : _shaders) shader->push_stages(shader_stages);
	const auto color_fmt
	    = window().surface().swapchain_capabilities().surface_format.format;

	const vk::StructureChain create_info {
		vk::GraphicsPipelineCreateInfo {
		    .stageCount = static_cast<uint32_t>(shader_stages.size()),
		    .pStages = shader_stages.data(),
		    .pVertexInputState = &vertex_input_info,
		    .pInputAssemblyState = &input_assembly_info,
		    .pTessellationState = nullptr,
		    .pViewportState = &viewport_state_info,
		    .pRasterizationState = &rasterizer_info,
		    .pMultisampleState = &multisampling_info,
		    .pDepthStencilState = nullptr,
		    .pColorBlendState = &color_blending_info,
		    .pDynamicState = &dynamic_state_info,
		    .layout = *_layout,
		},
		vk::PipelineRenderingCreateInfo {
		    .colorAttachmentCount = 1,
		    .pColorAttachmentFormats = &color_fmt,
		},
	};
	return vk::raii::Pipeline(renderer()->device(), nullptr,
	    create_info.get<vk::GraphicsPipelineCreateInfo>());
}

euler::util::Reference<euler::vulkan::Renderer>
euler::vulkan::Pipeline::renderer()
{
	return window().renderer();
}