/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_GRAPHICS_PIPELINE_H
#define EULER_VULKAN_GRAPHICS_PIPELINE_H

#include "euler/vulkan/fragment_shader.h"
#include "euler/vulkan/pipeline.h"
#include "euler/vulkan/vertex_shader.h"

namespace euler::vulkan {
class GraphicsPipeline final : public Pipeline {
public:
	// GraphicsPipeline(const util::Reference<Renderer> &r,
	//     const util::Reference<FragmentShader> &fragment_shader,
	//     const util::Reference<VertexShader> &vertex_shader);
	GraphicsPipeline(Window &w,
	    const std::vector<util::Reference<Shader>> &shaders)
	    : Pipeline(w, shaders)
	{
	}
};
} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_GRAPHICS_PIPELINE_H */
