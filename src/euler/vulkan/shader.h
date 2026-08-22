/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_SHADER_H
#define EULER_VULKAN_SHADER_H

#include <vulkan/vulkan_raii.hpp>

#include "euler/util/object.h"

namespace euler::vulkan {
class Renderer;

class Shader : public util::Object {
public:
	enum class Type {
		Compound,
		Vertex,
		Fragment,
		Compute,
	};
	virtual Type type() const = 0;
	virtual vk::ShaderStageFlagBits stage() const = 0;

	virtual size_t stage_count() const;
	virtual void push_stages(
	    std::vector<vk::PipelineShaderStageCreateInfo> &stages) const;

	Shader(Renderer *renderer,
	    vk::raii::ShaderModule &&module)
	    : _module(std::move(module))
	    , _renderer(renderer)
	{
	}

	const vk::raii::ShaderModule &
	module() const
	{
		return _module;
	}

protected:
	vk::raii::ShaderModule _module;
	util::WeakReference<Renderer> _renderer;
};

} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_SHADER_H */
