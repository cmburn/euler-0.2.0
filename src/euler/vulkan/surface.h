/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_SURFACE_H
#define EULER_VULKAN_SURFACE_H

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "euler/vulkan/swapchain.h"

namespace euler::vulkan {
class Window;
class Renderer;
class GraphicsPipeline;

class Surface final {
	friend class Renderer;

public:
	explicit Surface(Window &window);

	~Surface() = default;

	const vk::raii::SurfaceKHR &
	surface() const
	{
		return _surface;
	}

	vk::raii::SurfaceKHR &
	surface()
	{
		return _surface;
	}

	util::Reference<Renderer> renderer() const;
	vk::Extent2D extent() const;
	Swapchain &
	swapchain()
	{
		return _swapchain;
	}
	const Swapchain &
	swapchain() const
	{
		return _swapchain;
	}
	Swapchain::Capabilities &
	swapchain_capabilities()
	{
		return _swapchain_capabilities;
	}
	const Swapchain::Capabilities &
	swapchain_capabilities() const
	{
		return _swapchain_capabilities;
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

	void record_commands(const vk::raii::CommandBuffer &cmd,
	    uint32_t image_index);

private:
	Window &_window;
	vk::raii::SurfaceKHR _surface;
	vk::Extent2D _extent;
	Swapchain::Capabilities _swapchain_capabilities;
	Swapchain _swapchain;
};
} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_SURFACE_H */
