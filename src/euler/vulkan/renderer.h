/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_RENDERER_H
#define EULER_VULKAN_RENDERER_H

#include <vector>

#define VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL 0

#include <vk_mem_alloc.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

#include "euler/util/logger.h"
#include "euler/util/object.h"
#include "euler/util/state.h"

namespace euler::vulkan {
class Surface;

class Renderer final : public util::Object {
	friend class Surface;

public:
	Renderer(const util::Reference<util::State> &state);
	~Renderer() override;
	static void global_init();

	const vk::raii::Device &
	device() const
	{
		return _device;
	}
	vk::raii::Device &
	device()
	{
		return _device;
	}
	const vk::raii::Instance &
	instance() const
	{
		return _instance;
	}
	vk::raii::Instance &
	instance()
	{
		return _instance;
	}
	util::Reference<util::Logger>
	log() const
	{
		return _log;
	}

private:
	util::Reference<util::State> state();
	vk::raii::Instance create_instance();
	vk::raii::PhysicalDevice select_physical_device();
	uint32_t find_graphics_present_queue() const;
	uint32_t find_compute_queue() const;
	vk::raii::Device create_logical_device();
	VmaAllocator make_allocator();

	util::Reference<util::Logger> _log;
	util::WeakReference<util::State> _state;
	std::optional<uint32_t> _preferred_gpu;
	vk::raii::Context _context;
	vk::raii::Instance _instance;
	vk::raii::PhysicalDevice _physical_device;
	uint32_t _graphics_index;
	uint32_t _compute_index;
	vk::raii::Device _device;
	VmaAllocator _allocator;
	vk::raii::Queue _graphics_queue;
};
} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_RENDERER_H */
