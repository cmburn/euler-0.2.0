/* SPDX-License-Identifier: ISC */

#include "euler/vulkan/vertex_shader.h"

void
euler::vulkan::VertexShader::push_stages(
    std::vector<vk::PipelineShaderStageCreateInfo> &stages) const
{
	stages.push_back(vk::PipelineShaderStageCreateInfo {
	    .stage = vk::ShaderStageFlagBits::eVertex,
	    .module = *_module,
	    .pName = "main",
	});
}