/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_WINDOW_H
#define EULER_VULKAN_WINDOW_H

#include <vulkan/vulkan_raii.hpp>

#include "euler/graphics/window.h"
#include "euler/vulkan/renderer.h"
#include "euler/vulkan/surface.h"

typedef struct SDL_Window SDL_Window;

namespace euler::vulkan {
class Renderer;

class Window : public graphics::Window {
	friend class Renderer;
public:
	Window(const util::Reference<Renderer> &r, const char *title, int16_t w,
	    int16_t h, uint64_t flags);
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

	~Window() override;
	Window();
	Window(const util::Reference<Renderer> &, const char *title, int16_t w,
	    int16_t h, Flags flags = default_flags());


	[[nodiscard]] int16_t width() const override;
	[[nodiscard]] int16_t height() const override;
	[[nodiscard]] std::string_view title() const override;
	vk::raii::SurfaceKHR create_surface() const;
	void scissor(const ScissorCommand &cmd) override;
	void line(const LineCommand &cmd) override;
	void curve(const CurveCommand &cmd) override;
	void rect(const RectCommand &cmd) override;
	void circle(const CircleCommand &cmd) override;
	void arc(const ArcCommand &cmd) override;
	void triangle(const TriangleCommand &cmd) override;
	void polygon(const PolygonCommand &cmd) override;
	void text(const TextCommand &cmd) override;
	void image(const ImageCommand &cmd) override;
	util::Reference<graphics::Image> to_image() const override;
	[[nodiscard]] util::Reference<util::State> state() const override;
	util::Reference<Renderer> renderer() const
	{
		return _renderer;
	}

private:
	SDL_Window *_window;
	util::Reference<Renderer> _renderer;
	Surface _surface;
};
} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_WINDOW_H */
