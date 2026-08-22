/* SPDX-License-Identifier: ISC */

#include "euler/app/native/state.h"

#include <fstream>

#include <SDL3/SDL_timer.h>

#include <nlohmann/json.hpp>

#include "SDL3/SDL_oldnames.h"
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

const mrb_data_type State::TYPE = {
	.struct_name = "Euler::App::State",
	.dfree = [](mrb_state *, void *) {
		/* do nothing, if mrb is being destroyed than our top level
		 * state is too */
	}
};

euler::util::Reference<State>
State::get(const mrb_state *mrb)
{
	auto st = util::State::get(mrb);
	return static_cast<State *>(st.get());
}

State::State()
{
	Logger::global_init();
	vulkan::Renderer::global_init();
}

State::~State() = default;

State::Runtime
State::runtime() const
{
	return Runtime::Native;
}

const euler::util::RubyState &
State::rb() const
{
	return _rb;
}

euler::util::RubyState &
State::rb()
{
	return _rb;
}

RClass *
State::object_class() const
{
	return _rb.mrb()->object_class;
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
State::initialize(const std::string_view progname, const util::Config &config)
{
	static constexpr vulkan::Window::Flags flags = {};
	_progname = progname;
	if (config.title.has_value()) _title = config.title.value();
	_logger = util::make_reference<Logger>("euler", "engine",
	    config.severity.value_or(Logger::DEFAULT_SEVERITY));
	_renderer
	    = util::make_reference<vulkan::Renderer>(util::Reference(this));
	_window = _renderer->create_window(_title.c_str(), 800, 600, flags);
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
	SDL_Event e;
	if (SDL_PollEvent(&e)) {
		if (e.type == SDL_EVENT_QUIT) {
			exit_code = EXIT_SUCCESS;
			return false;
		}
	}
	_renderer->draw();
	return true;
}
