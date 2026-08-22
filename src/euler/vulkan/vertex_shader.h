/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_VERTEX_SHADER_H
#define EULER_VULKAN_VERTEX_SHADER_H

#include "euler/vulkan/shader.h"

namespace euler::vulkan {
class VertexShader final : public Shader {
public:
	VertexShader(Renderer *renderer,
	    vk::raii::ShaderModule &&module)
	    : Shader(renderer, std::move(module))
	{
	}

	[[nodiscard]] Type type() const override { return Type::Vertex; }
	[[nodiscard]] size_t stage_count() const override { return 1; }
	void push_stages(std::vector<vk::PipelineShaderStageCreateInfo> &stages)
	    const override;
	vk::ShaderStageFlagBits stage() const override
	{
		return vk::ShaderStageFlagBits::eVertex;
	}
};
} /* namespace euler::vulkan */


#endif /* EULER_VULKAN_VERTEX_SHADER_H */

