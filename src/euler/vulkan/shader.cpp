/* SPDX-License-Identifier: ISC */

#include "euler/vulkan/shader.h"

size_t
euler::vulkan::Shader::stage_count() const
{
	return 1;
}

void
euler::vulkan::Shader::push_stages(
    std::vector<vk::PipelineShaderStageCreateInfo> &stages) const
{
	stages.push_back(vk::PipelineShaderStageCreateInfo {
	    .stage = stage(),
	    .module = *_module,
	    .pName = "main",
	});
}