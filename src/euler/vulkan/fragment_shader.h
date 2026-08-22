/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_FRAGMENT_SHADER_H
#define EULER_VULKAN_FRAGMENT_SHADER_H

#include "euler/vulkan/shader.h"

namespace euler::vulkan {
class FragmentShader final : public Shader {
public:
	FragmentShader(Renderer *renderer,
	    vk::raii::ShaderModule &&module)
	    : Shader(renderer, std::move(module))
	{
	}

	Type
	type() const override
	{
		return Type::Fragment;
	}

	vk::ShaderStageFlagBits
	stage() const override
	{
		return vk::ShaderStageFlagBits::eFragment;
	}
};
} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_FRAGMENT_SHADER_H */
