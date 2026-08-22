/* SPDX-License-Identifier: ISC */

#ifndef EULER_APP_STATE_H
#define EULER_APP_STATE_H

#include "euler/util/ext.h"
#include "euler/util/object.h"

#if defined(EULER_DRAGONRUBY)
#include "euler/app/dragonruby/state.h"
#define EULER_APP_NAMESPACE euler::app::dragonruby
#elif defined(EULER_NATIVE)
#include "euler/app/native/state.h"
#define EULER_APP_NAMESPACE euler::app::native
#else
#error "No app backend defined"
#endif

namespace euler::app {
class State final : public EULER_APP_NAMESPACE::State {

public:
	~State() override;
	State();
	[[nodiscard]] nthread_t available_threads() const override;
	[[nodiscard]] mrb_value gv_state() override;
	bool initialize(std::string_view progname,
	    const util::Config &config) override;
	[[nodiscard]] tick_t last_tick() const override;
	[[nodiscard]] float fps() const override;
	[[nodiscard]] tick_t total_ticks() const override;
	void tick() override;
	[[nodiscard]] RClass *object_class() const override;
	void *unwrap(mrb_value value, const mrb_data_type *type) override;

	[[nodiscard]] const Modules &
	modules() const override
	{
		return _modules;
	}

	[[nodiscard]] Modules &
	modules() override
	{
		return _modules;
	}

	[[nodiscard]] Phase phase() const override;
	void set_phase(Phase phase) override;
	mrb_value self_value() override;

	util::Version app_version() const override;
	util::Version engine_version() const override;

	[[nodiscard]] const util::Config &
	config() const override
	{
		return _config;
	}

protected:
	[[nodiscard]] const mrb_data_type *
	data_type() const override
	{
		return &TYPE;
	}

private:
	void initialize_self();

	util::Config _config;
	bool _initialized_self = false;
	Phase _phase = Phase::Update;
	float _fps = 0;
	tick_t _last_tick = 0;
	tick_t _tick = 0;
	tick_t _total_ticks = 0;
	// frames elapsed over the last second
	tick_t _frames = 0;
	tick_t _last_frame_tick = 0;
	tick_t _last_frame_total_ticks = 0;
	Modules _modules = {};
	mrb_value _self_value = mrb_nil_value();
	util::Version _app_version;
};

} /* namespace euler::app */

#endif /* EULER_APP_STATE_H */
