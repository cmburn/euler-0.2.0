/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_COMPUTE_SHADER_H
#define EULER_VULKAN_COMPUTE_SHADER_H

#include "euler/vulkan/shader.h"

namespace euler::vulkan {
class ComputeShader final : public Shader {
public:
	ComputeShader(Renderer *renderer,
	    vk::raii::ShaderModule &&module)
	    : Shader(renderer, std::move(module))
	{
	}

	Type
	type() const override
	{
		return Type::Compute;
	}

	size_t
	stage_count() const override
	{
		return 1;
	}

	vk::ShaderStageFlagBits
	stage() const override
	{
		return vk::ShaderStageFlagBits::eCompute;
	}
};
} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_COMPUTE_SHADER_H */
