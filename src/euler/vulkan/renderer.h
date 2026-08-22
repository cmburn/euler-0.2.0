/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_RENDERER_H
#define EULER_VULKAN_RENDERER_H

#include <unordered_map>
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
#include "euler/vulkan/shader.h"
#include "euler/vulkan/window.h"

#ifndef EULER_VULKAN_MAX_FRAMES_IN_FLIGHT
#define EULER_VULKAN_MAX_FRAMES_IN_FLIGHT 3
#endif

namespace euler::vulkan {
class Surface;
class GraphicsPipeline;
class FragmentShader;
class VertexShader;
class CompoundShader;
class ComputeShader;
class Shader;
class Swapchain;
class Window;

class Renderer final : public util::Object {
	friend class Shader;
	friend void Window::close();
	friend void Window::set_title(const char *);

public:
	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT
	    = EULER_VULKAN_MAX_FRAMES_IN_FLIGHT;

	Renderer(const util::Reference<util::State> &state);
	~Renderer() override;
	static void global_init();
	uint32_t window_count() const;

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
	const vk::raii::PhysicalDevice &
	physical_device() const
	{
		return _physical_device;
	}
	vk::raii::PhysicalDevice &
	physical_device()
	{
		return _physical_device;
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

	const vk::raii::Context &
	context() const
	{
		return _context;
	}

	util::Reference<Window> create_window(const char *title, int16_t w,
	    int16_t h, Window::Flags flags = Window::default_flags());
	util::Reference<FragmentShader> load_fragment_shader(
	    std::string_view path);
	util::Reference<VertexShader> load_vertex_shader(std::string_view path);
	util::Reference<ComputeShader> load_compute_shader(
	    std::string_view path);
	util::Reference<CompoundShader> load_compound_shader(
	    std::string_view path,
	    const std::vector<std::pair<std::string_view, Shader::Type>>
	        &types);
	util::Reference<FragmentShader> fragment_shader() const;
	util::Reference<VertexShader> vertex_shader() const;
	void draw();

private:
	typedef std::array<vk::raii::CommandBuffer, MAX_FRAMES_IN_FLIGHT>
	    CommandBufferSet;
	template <typename T>
	util::Reference<T>
	load_shader(const std::string_view path)
	{
		const auto data = load_shader_data(path);
		const vk::ShaderModuleCreateInfo create_info = {
			.codeSize = data.size() * sizeof(uint32_t),
			.pCode = data.data(),
		};
		vk::raii::ShaderModule module(_device, create_info);
		return util::make_reference<T>(this, std::move(module));
	}

	util::Reference<util::State> state() const;
	vk::raii::Instance create_instance();
	vk::raii::PhysicalDevice select_physical_device();
	static CommandBufferSet make_command_buffer_set();
	uint32_t find_graphics_present_queue() const;
	uint32_t find_compute_queue() const;
	vk::raii::Device create_logical_device();
	VmaAllocator make_allocator();
	[[nodiscard]] std::vector<uint32_t> load_shader_data(
	    std::string_view path) const;
	vk::raii::CommandPool create_command_pool();
	std::vector<CommandBufferSet> create_command_buffers();
	void rebuild_command_buffers();
	void rebuild_window_cache();
	void record_command_buffer(const vk::raii::CommandBuffer &cmd,
		Window &window, uint32_t image_index);
	void record_command_buffers();

	struct ImageLayoutTransition {
		Swapchain &swapchain;
		uint32_t image_index;
		vk::ImageLayout old_layout;
		vk::ImageLayout new_layout;
		vk::AccessFlags src_access_mask;
		vk::AccessFlags dst_access_mask;
		vk::PipelineStageFlags src_stage_mask;
		vk::PipelineStageFlags dst_stage_mask;
	};
	void transition_image_layout(const vk::raii::CommandBuffer &buf,
		const ImageLayoutTransition &ilt);

	util::Reference<util::Logger> _log;
	util::WeakReference<util::State> _state;

	vk::raii::Context _context;
	vk::raii::Instance _instance;
	vk::raii::PhysicalDevice _physical_device;
	uint32_t _graphics_index;
	uint32_t _compute_index;
	vk::raii::Device _device;
	VmaAllocator _allocator;
	vk::raii::Queue _graphics_queue;
	util::Reference<FragmentShader> _fragment_shader;
	util::Reference<VertexShader> _vertex_shader;
	std::unordered_map<std::string, uint32_t> _window_indices;
	std::vector<util::WeakReference<Window>> _windows;
	vk::raii::CommandPool _command_pool;
	std::vector<CommandBufferSet> _command_buffers;
};
} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_RENDERER_H */
