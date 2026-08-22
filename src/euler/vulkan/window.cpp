/* SPDX-License-Identifier: ISC */

#include "euler/vulkan/window.h"

#include <unordered_set>

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

#include "euler/vulkan/graphics_pipeline.h"
#include "euler/vulkan/renderer.h"

using euler::vulkan::Window;

static uint64_t
to_sdl(const Window::Flags flags)
{
	uint64_t out = SDL_WINDOW_VULKAN;
	if (flags.fullscreen) out |= SDL_WINDOW_FULLSCREEN;
	if (flags.hidden) out |= SDL_WINDOW_HIDDEN;
	if (flags.borderless) out |= SDL_WINDOW_BORDERLESS;
	if (flags.resizable) out |= SDL_WINDOW_RESIZABLE;
	if (flags.minimized) out |= SDL_WINDOW_MINIMIZED;
	if (flags.maximized) out |= SDL_WINDOW_MAXIMIZED;
	if (flags.mouse_grabbed) out |= SDL_WINDOW_MOUSE_GRABBED;
	if (flags.input_focus) out |= SDL_WINDOW_INPUT_FOCUS;
	if (flags.modal) out |= SDL_WINDOW_MODAL;
	if (flags.high_pixel_density) out |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
	if (flags.mouse_capture) out |= SDL_WINDOW_MOUSE_CAPTURE;
	if (flags.always_on_top) out |= SDL_WINDOW_ALWAYS_ON_TOP;
	if (flags.utility) out |= SDL_WINDOW_UTILITY;
	if (flags.tooltip) out |= SDL_WINDOW_TOOLTIP;
	if (flags.popup_menu) out |= SDL_WINDOW_POPUP_MENU;
	if (flags.keyboard_grabbed) out |= SDL_WINDOW_KEYBOARD_GRABBED;
	if (flags.window_transparent) out |= SDL_WINDOW_TRANSPARENT;
	if (flags.not_focusable) out |= SDL_WINDOW_NOT_FOCUSABLE;
	return out;
}

Window::Window(const util::Reference<Renderer> &r, const char *title,
    const int16_t w, const int16_t h, const uint64_t flags)
    : _window(SDL_CreateWindow(title, w, h, flags))
    , _renderer(r)
    , _surface(*this)
{
}

Window::~Window()
{
	if (_window == nullptr) return;
	SDL_DestroyWindow(_window);
	_window = nullptr;
}

Window::Window(const util::Reference<Renderer> &r, const char *title,
    const int16_t w, const int16_t h, const Flags flags)
    : Window(r, title, w, h, to_sdl(flags))
{
}

int16_t
Window::width() const
{
	int w, h;
	SDL_GetWindowSizeInPixels(_window, &w, &h);
	return static_cast<int16_t>(w);
}

int16_t
Window::height() const
{
	int w, h;
	SDL_GetWindowSizeInPixels(_window, &w, &h);
	return static_cast<int16_t>(h);
}

std::string_view
Window::title() const
{
	return SDL_GetWindowTitle(_window);
}
vk::raii::SurfaceKHR
Window::create_surface() const
{
	VkSurfaceKHR surface;
	if (SDL_Vulkan_CreateSurface(_window, *renderer()->instance(), nullptr,
	        &surface)) {
		return vk::raii::SurfaceKHR(renderer()->instance(), surface);
	}
	renderer()->log()->fatal("Failed to create Vulkan surface: {}",
	    SDL_GetError());
}

void
Window::scissor(const ScissorCommand &)
{
}

void
Window::line(const LineCommand &)
{
}

void
Window::curve(const CurveCommand &)
{
}

void
Window::rect(const RectCommand &)
{
}

void
Window::circle(const CircleCommand &)
{
}

void
Window::arc(const ArcCommand &)
{
}

void
Window::triangle(const TriangleCommand &)
{
}

void
Window::polygon(const PolygonCommand &)
{
}

void
Window::text(const TextCommand &)
{
}

void
Window::image(const ImageCommand &)
{
}

euler::util::Reference<euler::graphics::Image>
Window::to_image() const
{
	return nullptr;
}

euler::util::Reference<euler::util::State>
Window::state() const
{
	return nullptr;
}

euler::util::Reference<euler::vulkan::Renderer>
Window::renderer() const
{
	return _renderer;
}

void
Window::close()
{
	_active = false;
	const auto str = std::string(title());
	const auto idx = _renderer->_window_indices.at(str);
	_renderer->_window_indices.erase(str);
	_renderer->_windows.at(idx) = nullptr;
}

void
Window::set_title(const char *title)
{
	SDL_SetWindowTitle(_window, title);
	const auto str = std::string(title);
	const auto idx = _renderer->_window_indices.at(str);
	_renderer->_window_indices.erase(str);
	_renderer->_window_indices.emplace(str, idx);
}
