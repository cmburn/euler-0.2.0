/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_COMPOUND_SHADER_H
#define EULER_VULKAN_COMPOUND_SHADER_H

#include <unordered_map>

#include "euler/vulkan/shader.h"

namespace euler::vulkan {
class CompoundShader final : public Shader {
public:
	Type
	type() const override
	{
		return Type::Compound;
	}
	size_t stage_count() const override { return _types.size(); }
	void push_stages(std::vector<vk::PipelineShaderStageCreateInfo> &stages)
	    const override;

private:
	std::unordered_map<std::string, Type> _types;
};
} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_COMPOUND_SHADER_H */
