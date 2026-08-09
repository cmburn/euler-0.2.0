/* SPDX-License-Identifier: ISC */

#include "euler/app/native/state.h"

#include <SDL3/SDL_timer.h>

#include "euler/graphics/image_loader.h"
#include "euler/vulkan/window.h"

#ifndef ENGINE_VERSION_MAJOR
#define ENGINE_VERSION_MAJOR 0
#endif

#ifndef ENGINE_VERSION_MINOR
#define ENGINE_VERSION_MINOR 1
#endif

#ifndef ENGINE_VERSION_PATCH
#define ENGINE_VERSION_PATCH 0
#endif

static constexpr euler::util::Version ENGINE_VERSION(ENGINE_VERSION_MAJOR,
    ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH);

using euler::app::native::State;

euler::util::Reference<State>
State::get(const mrb_state *mrb)
{
	auto st = util::State::get(mrb);
	return static_cast<State *>(st.get());
}

State::State() { _mrb = util::make_reference<RubyState>(); }

State::Runtime
State::runtime() const
{
	return Runtime::Native;
}

euler::util::Reference<euler::util::RubyState>
State::mrb() const
{
	return _mrb;
}

RClass *
State::object_class() const
{
	return _mrb->mrb()->object_class;
}

euler::util::Reference<euler::util::Logger>
State::log() const
{
	return _logger;
}

State::tick_t
State::ticks() const
{
	return SDL_GetTicks();
}

float
State::dt() const
{
	return static_cast<float>(ticks() - last_tick()) / 1000.0f;
}

const std::string &
State::progname() const
{
	return _progname;
}

const std::string &
State::title() const
{
	return _title;
}

bool
State::initialize(const util::Config &config)
{
	static constexpr vulkan::Window::Flags flags = {};
	_progname = config.progname;
	_title = config.title;
	_logger
	    = util::make_reference<Logger>("euler", "engine", config.severity);
	_renderer
	    = util::make_reference<vulkan::Renderer>(util::Reference(this));
	_window = util::make_reference<vulkan::Window>(_renderer,
	    _title.c_str(), 800, 600, flags);
	return true;
}

euler::util::Reference<euler::graphics::ImageLoader>
State::image_loader()
{
	/* TODO */
	return nullptr;
}

std::optional<uint32_t>
State::preferred_gpu() const
{
	return _preferred_gpu;
}

bool
State::loop(int &exit_code)
{
	(void)exit_code;
	return true;
}

euler::util::Config
euler::app::native::parse_config(int, const char **)
{
	util::Config config;
	/* TODO: parse command line arguments */
	return config;
}
