/* SPDX-License-Identifier: ISC */

#include "euler/vulkan/surface.h"

#include <unordered_set>

#include <SDL3/SDL_video.h>

#include "euler/vulkan/renderer.h"
#include "euler/vulkan/window.h"
#include "euler/vulkan/graphics_pipeline.h"

using euler::vulkan::Surface;

vk::Extent2D
Surface::extent() const
{
	return vk::Extent2D {
		.width = static_cast<uint32_t>(_window.width()),
		.height = static_cast<uint32_t>(_window.height()),
	};
}

void
Surface::record_commands(const vk::raii::CommandBuffer &, uint32_t)
{
	/* TODO */
}

Surface::Surface(Window &window)
    : _window(window)
    , _surface(_window.create_surface())
    , _swapchain_capabilities(renderer()->physical_device(), _surface)
    , _swapchain(*this, _swapchain_capabilities)
{
}

euler::util::Reference<euler::vulkan::Renderer>
Surface::renderer() const
{
	return _window.renderer();
}