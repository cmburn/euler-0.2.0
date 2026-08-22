/* SPDX-License-Identifier: ISC */

#include "euler/vulkan/compound_shader.h"

void
euler::vulkan::CompoundShader::push_stages(
    std::vector<vk::PipelineShaderStageCreateInfo> &stages) const
{
	for (const auto &[name, type] : _types) {
		vk::ShaderStageFlagBits stage_flag;
		switch (type) {
		case Type::Vertex:
			stage_flag = vk::ShaderStageFlagBits::eVertex;
			break;
		case Type::Fragment:
			stage_flag = vk::ShaderStageFlagBits::eFragment;
			break;
		case Type::Compute:
			stage_flag = vk::ShaderStageFlagBits::eCompute;
			break;
		default: {
			const auto msg = std::format(
			    "Unexpected shader type for state '{}': {}", name,
			    static_cast<int>(type));
			throw std::runtime_error(msg);
		}
		}
		stages.push_back(vk::PipelineShaderStageCreateInfo {
		    .stage = stage_flag,
		    .module = *_module,
		    .pName = name.c_str(),
		});
	}
}