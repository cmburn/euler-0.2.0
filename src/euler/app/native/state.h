/* SPDX-License-Identifier: ISC */

#ifndef EULER_APP_NATIVE_STATE_H
#define EULER_APP_NATIVE_STATE_H

#include "euler/app/native/logger.h"
#include "euler/app/native/ruby_state.h"
#include "euler/util/config.h"
#include "euler/util/state.h"
#include "euler/util/version.h"
#include "euler/vulkan/renderer.h"
#include "euler/vulkan/surface.h"
#include "euler/vulkan/window.h"

namespace euler::app::native {

class State : public util::State {
	BIND_MRUBY_DATA("Euler::App::State", State, app.state);
public:
	static util::Reference<State> get(const mrb_state *mrb);
	State();
	~State() override;
	[[nodiscard]] Runtime runtime() const override;
	[[nodiscard]] const util::RubyState &rb() const override;
	[[nodiscard]] util::RubyState &rb() override;
	[[nodiscard]] RClass *object_class() const override;
	[[nodiscard]] util::Reference<util::Logger> log() const override;
	[[nodiscard]] tick_t ticks() const override;
	[[nodiscard]] float dt() const override;

	[[nodiscard]] const std::string &progname() const override;
	[[nodiscard]] const std::string &title() const override;
	[[nodiscard]] bool initialize(std::string_view progname,
	    const util::Config &config) override;
	[[nodiscard]] util::Reference<graphics::ImageLoader>
	image_loader() override;
	[[nodiscard]] std::optional<uint32_t> preferred_gpu() const override;

	bool loop(int &exit_code);

private:
	RubyState _rb;
	util::Reference<Logger> _logger;
	nthread_t _max_threads = std::thread::hardware_concurrency();
	std::string _progname;
	std::string _title;
	std::optional<uint32_t> _preferred_gpu;
	util::Reference<vulkan::Renderer> _renderer;
	util::Reference<vulkan::Window> _window;
};

util::Config parse_config(int argc, const char **argv);

} /* namespace euler::app::native */

#endif /* EULER_APP_NATIVE_STATE_H */
