/* SPDX-License-Identifier: ISC */

#include "euler/app/state.h"

#include <cassert>

#include "euler/physics/ext.h"
#include "euler/util/ext.h"
#include "euler/util/logger.h"

#ifndef EULER_GV_STATE
#define EULER_GV_STATE app
#endif

static constexpr auto GV_STATE_SYM = "$" MRB_STRINGIZE(EULER_GV_STATE);

static constexpr euler::util::Version ENGINE_VERSION(0, 1, 0);

using euler::app::State;

static euler::util::Reference<State>
from_mrb(mrb_state *mrb, mrb_value self)
{
	return euler::util::State::get(mrb)->unwrap<State>(self);
}

State::~State() = default;
State::State() = default;

euler::util::State::nthread_t
State::available_threads() const
{
	return std::thread::hardware_concurrency();
}

mrb_value
State::gv_state()
{
	return rb().gv_get(rb().intern_cstr(GV_STATE_SYM));
}

static mrb_value
state_allocate(mrb_state *mrb, const mrb_value self)
{
	auto state = State::get(mrb);
	assert(state != nullptr);
	const auto cl = mrb_class_ptr(self);
	const auto obj = Data_Wrap_Struct(mrb, cl, &State::TYPE, state.wrap());
	return mrb_obj_value(obj);
}

static mrb_value
state_phase(mrb_state *mrb, const mrb_value self)
{
	const auto state = from_mrb(mrb, self);
	switch (state->phase()) {
	case State::Phase::Load: return EULER_SYM_VAL(load);
	case State::Phase::Input: return EULER_SYM_VAL(input);
	case State::Phase::Update: return EULER_SYM_VAL(update);
	case State::Phase::Draw: return EULER_SYM_VAL(draw);
	case State::Phase::Quit: return EULER_SYM_VAL(quit);
	default: std::unreachable();
	}
}

static mrb_value
state_ticks(mrb_state *mrb, const mrb_value self)
{
	const auto state = from_mrb(mrb, self);
	return state->rb().int_value(state->ticks());
}

static mrb_value
state_fps(mrb_state *mrb, const mrb_value self)
{
	const auto state = from_mrb(mrb, self);
	return state->rb().float_value(state->fps());
}

static mrb_value
state_dt(mrb_state *mrb, const mrb_value self)
{
	const auto state = from_mrb(mrb, self);
	return state->rb().float_value(state->dt());
}

static mrb_value
state_progname(mrb_state *mrb, const mrb_value self)
{
	const auto state = from_mrb(mrb, self);
	return state->rb().str_new_cstr(state->progname().c_str());
}

static mrb_value
state_title(mrb_state *mrb, const mrb_value self)
{
	const auto state = from_mrb(mrb, self);
	return state->rb().str_new_cstr(state->title().c_str());
}

bool
State::initialize(const std::string_view progname, const util::Config &config)
{
	rb().mrb()->ud = util::WeakReference(this).wrap();
	_config = config;
	initialize_self();
	if (!EULER_APP_NAMESPACE::State::initialize(progname, config)) return false;
	log()->debug("Initializing core modules");
	const auto self = util::Reference(this);
	auto &mods = modules();
	assert(mods.mod != nullptr);

	// mods.graphics.mod = graphics::init(self, mods.mod);
	// mods.gui.mod = gui::init(self, mods.mod);
	// #ifdef EULER_MATH
	// 	mods.math.mod = math::init(self, mods.mod);
	// #endif
#ifdef EULER_PHYSICS
	mods.physics.mod = physics::init(self, mods.mod);
#endif
	mods.util.mod = util::init(self, mods.mod);
	log()->debug("Core modules initialized");

	// if (!EULER_APP_NAMESPACE::State::initialize(argc, argv)) return
	// false; auto self = util::Reference(this); 	util::init(self, mod);
	// #ifdef EULER_MATH
	// 	math::init(self, mod);
	// #endif
	// #ifdef EULER_PHYSICS
	// 	physics::init(self, mod);
	// #endif
	return true;
}

euler::util::State::tick_t
State::last_tick() const
{
	return _last_tick;
}

float
State::fps() const
{
	return _fps;
}

euler::util::State::tick_t
State::total_ticks() const
{
	return _total_ticks;
}

void
State::tick()
{
	_last_tick = _tick;
	_tick = ticks();
	++_total_ticks;
	if (_tick - _last_frame_tick < 1000) return;
	const auto frames = _total_ticks - _last_frame_total_ticks;
	const auto milliseconds = _tick - _last_frame_tick;
	const auto seconds = static_cast<double>(milliseconds) / 1000.0;
	_fps = static_cast<float>(static_cast<double>(frames) / seconds);
	_last_frame_tick = _tick;
	_last_frame_total_ticks = _total_ticks;
}

RClass *
State::object_class() const
{
	return this->rb().mrb()->object_class;
}

void *
State::unwrap(const mrb_value value, const mrb_data_type *type)
{
	return rb().data_check_get_ptr(value, type);
}
State::Phase
State::phase() const
{
	return _phase;
}
void
State::set_phase(Phase phase)
{
	_phase = phase;
}

mrb_value
State::self_value()
{
	// assert(_initialized_self);
	// return _self_value;
	auto gv = gv_state();
	return mrb_nil_p(gv) ? _self_value : gv;
}

euler::util::Version
State::app_version() const
{
	return _app_version;
}

euler::util::Version
State::engine_version() const
{
	return ENGINE_VERSION;
}

void
State::initialize_self()
{
	if (_initialized_self) return;
	const auto mod = rb().define_module("Euler");
	modules().mod = mod;
	const auto util_mod = rb().define_module_under(mod, "Util");
	modules().util.mod = util_mod;
	const auto cls
	    = rb().define_class_under(util_mod, "State", object_class());
	MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);
	modules().app.state = cls;
	rb().define_class_method(cls, "allocate", state_allocate,
	    MRB_ARGS_NONE());
	rb().define_method(cls, "phase", state_phase, MRB_ARGS_NONE());
	rb().define_method(cls, "ticks", state_ticks, MRB_ARGS_NONE());
	rb().define_method(cls, "fps", state_fps, MRB_ARGS_NONE());
	rb().define_method(cls, "dt", state_dt, MRB_ARGS_NONE());
	rb().define_method(cls, "progname", state_progname, MRB_ARGS_NONE());
	rb().define_method(cls, "title", state_title, MRB_ARGS_NONE());
	const auto ptr = util::WeakReference(this).wrap();
	const auto data = rb().data_object_alloc(cls, ptr, &State::TYPE);
	_self_value = mrb_obj_value(data);
	_initialized_self = true;
}
