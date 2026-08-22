/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_PIPELINE_H
#define EULER_VULKAN_PIPELINE_H

#include <vector>
#include <vulkan/vulkan_raii.hpp>

#include "euler/util/object.h"
#include "euler/vulkan/shader.h"

namespace euler::vulkan {
class Renderer;
class Window;

class Pipeline : public util::Object {
protected:
	Pipeline(Window &window,
	    const std::vector<util::Reference<Shader>> &shaders);

public:
	~Pipeline() override = default;

	const vk::raii::Pipeline &
	pipeline() const
	{
		return _pipeline;
	}

	vk::raii::Pipeline &
	pipeline()
	{
		return _pipeline;
	}

	const vk::raii::PipelineLayout &
	layout() const
	{
		return _layout;
	}

	vk::raii::PipelineLayout &
	layout()
	{
		return _layout;
	}

	Window &
	window()
	{
		return _window;
	}

	const Window &
	window() const
	{
		return _window;
	}

private:
	vk::raii::PipelineLayout create_layout();
	vk::raii::Pipeline create_pipeline();

protected:
	util::Reference<Renderer> renderer();
	Window &_window;
	std::vector<util::Reference<Shader>> _shaders;
	vk::raii::PipelineLayout _layout;
	vk::raii::Pipeline _pipeline;
};
} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_PIPELINE_H */
