/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_SURFACE_H
#define EULER_VULKAN_SURFACE_H

#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "euler/vulkan/swapchain.h"


namespace euler::vulkan {
class Window;
class Renderer;

class Surface final {
	friend class Renderer;

public:
	Surface(Window *window);

	struct Flags {
		bool fullscreen : 1 = false;
		bool hidden : 1 = false;
		bool borderless : 1 = false;
		bool resizable : 1 = false;
		bool minimized : 1 = false;
		bool maximized : 1 = false;
		bool mouse_grabbed : 1 = false;
		bool input_focus : 1 = false;
		bool modal : 1 = false;
		bool high_pixel_density : 1 = false;
		bool mouse_capture : 1 = false;
		bool always_on_top : 1 = false;
		bool utility : 1 = false;
		bool tooltip : 1 = false;
		bool popup_menu : 1 = false;
		bool keyboard_grabbed : 1 = false;
		bool window_transparent : 1 = false;
		bool not_focusable : 1 = false;
	};

	static Flags
	default_flags()
	{
		return Flags {};
	}

	~Surface() = default;

	const vk::raii::SurfaceKHR &surface() const
	{
		return _surface;
	}

	vk::raii::SurfaceKHR &surface()
	{
		return _surface;
	}

	util::Reference<Renderer> renderer() const;
	vk::Extent2D extent() const;

private:
	Window *_window;
	vk::raii::SurfaceKHR _surface;
	vk::Extent2D _extent;
	Swapchain::Capabilities _swapchain_capabilities;
	Swapchain _swapchain;
};
} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_SURFACE_H */
