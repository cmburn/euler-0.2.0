/* SPDX-License-Identifier: ISC */

#include "euler/physics/wheel_joint.h"

#include <box2d/box2d.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/hash.h>

#include "euler/physics/body.h"
#include "euler/physics/joint.h"
#include "euler/physics/util.h"
#include "euler/physics/world.h"
#include "euler/util/state.h"

using euler::physics::WheelJoint;

static mrb_value
wheel_joint_enable_spring(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	mrb_bool flag;
	state->rb().get_args("b", &flag);
	joint->enable_spring(flag);
	return mrb_nil_value();
}

static mrb_value
wheel_joint_is_spring_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	const bool flag = joint->is_spring_enabled();
	return mrb_bool_value(flag);
}

static mrb_value
wheel_joint_set_spring_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	mrb_float hertz;
	state->rb().get_args("f", &hertz);
	joint->set_spring_hertz((float)hertz);
	return mrb_nil_value();
}

static mrb_value
wheel_joint_spring_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	const float hertz = joint->spring_hertz();
	return state->rb().float_value(hertz);
}

static mrb_value
wheel_joint_set_spring_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	mrb_float damping_ratio;
	state->rb().get_args("f", &damping_ratio);
	joint->set_spring_damping_ratio((float)damping_ratio);
	return mrb_nil_value();
}

static mrb_value
wheel_joint_spring_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	const float damping_ratio = joint->spring_damping_ratio();
	return state->rb().float_value(damping_ratio);
}

static mrb_value
wheel_joint_enable_limit(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	mrb_bool flag;
	state->rb().get_args("b", &flag);
	joint->enable_limit(flag);
	return mrb_nil_value();
}

static mrb_value
wheel_joint_is_limit_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	const bool flag = joint->is_limit_enabled();
	return mrb_bool_value(flag);
}

static mrb_value
wheel_joint_set_limits(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	mrb_value arr = mrb_nil_value();
	state->rb().get_args("A", &arr);
	const b2Vec2 limits = euler::physics::value_to_b2_vec(mrb, arr);
	joint->set_limits(limits.x, limits.y);
	return mrb_nil_value();
}

static mrb_value
wheel_joint_limits(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	const auto [lower_limit, upper_limit] = joint->limits();
	const mrb_value out = state->rb().ary_new_capa(2);
	state->rb().ary_push(out, state->rb().float_value(lower_limit));
	state->rb().ary_push(out, state->rb().float_value(upper_limit));
	return out;
}

static mrb_value
wheel_joint_enable_motor(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	mrb_bool flag;
	state->rb().get_args("b", &flag);
	joint->enable_motor(flag);
	return mrb_nil_value();
}

static mrb_value
wheel_joint_is_motor_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	const bool flag = joint->is_motor_enabled();
	return mrb_bool_value(flag);
}

static mrb_value
wheel_joint_set_motor_speed(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	mrb_float motor_speed;
	state->rb().get_args("f", &motor_speed);
	joint->set_motor_speed((float)motor_speed);
	return mrb_nil_value();
}

static mrb_value
wheel_joint_motor_speed(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	const float motor_speed = joint->motor_speed();
	return state->rb().float_value(motor_speed);
}

static mrb_value
wheel_joint_set_max_motor_torque(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	mrb_float max_torque;
	state->rb().get_args("f", &max_torque);
	joint->set_max_motor_torque((float)max_torque);
	return mrb_nil_value();
}

static mrb_value
wheel_joint_max_motor_torque(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	const float max_torque = joint->max_motor_torque();
	return state->rb().float_value(max_torque);
}

static mrb_value
wheel_joint_motor_torque(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WheelJoint>(self);
	const float torque = joint->motor_torque();
	return state->rb().float_value(torque);
}

static RClass *
wheel_joint_init(mrb_state *mrb, RClass *mod, RClass *super)
{
	const auto state = euler::util::State::get(mrb);
	RClass *joint
	    = state->rb().define_class_under(mod, "WheelJoint", super);
	state->rb().define_method(joint,
	    "spring_enabled=", wheel_joint_enable_spring, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "spring_enabled",
	    wheel_joint_is_spring_enabled, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "spring_hertz=", wheel_joint_set_spring_hertz, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "spring_hertz",
	    wheel_joint_spring_hertz, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "spring_damping_ratio=", wheel_joint_set_spring_damping_ratio,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "spring_damping_ratio",
	    wheel_joint_spring_damping_ratio, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "limit_enabled=", wheel_joint_enable_limit, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "limit_enabled",
	    wheel_joint_is_limit_enabled, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "limits=", wheel_joint_set_limits,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "limits", wheel_joint_limits,
	    MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "motor_enabled=", wheel_joint_enable_motor, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "motor_enabled",
	    wheel_joint_is_motor_enabled, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "motor_speed=", wheel_joint_set_motor_speed, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "motor_speed",
	    wheel_joint_motor_speed, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "max_motor_torque=", wheel_joint_set_max_motor_torque,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "max_motor_torque",
	    wheel_joint_max_motor_torque, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "motor_torque",
	    wheel_joint_motor_torque, MRB_ARGS_REQ(0));
	return joint;
}

RClass *
WheelJoint::init(const util::Reference<util::State> &state, RClass *mod,
    RClass *super)
{
	return wheel_joint_init(state->rb().mrb(), mod, super);
}

void
WheelJoint::enable_spring(bool flag)
{
	b2WheelJoint_EnableSpring(_id, flag);
}

bool
WheelJoint::is_spring_enabled() const
{
	return b2WheelJoint_IsSpringEnabled(_id);
}

void
WheelJoint::set_spring_hertz(float hertz)
{
	b2WheelJoint_SetSpringHertz(_id, hertz);
}

float
WheelJoint::spring_hertz() const
{
	return b2WheelJoint_GetSpringHertz(_id);
}

void
WheelJoint::set_spring_damping_ratio(float ratio)
{
	b2WheelJoint_SetSpringDampingRatio(_id, ratio);
}

float
WheelJoint::spring_damping_ratio() const
{
	return b2WheelJoint_GetSpringDampingRatio(_id);
}

void
WheelJoint::enable_limit(bool flag)
{
	b2WheelJoint_EnableLimit(_id, flag);
}

bool
WheelJoint::is_limit_enabled() const
{
	return b2WheelJoint_IsLimitEnabled(_id);
}

void
WheelJoint::set_limits(float lower, float upper)
{
	b2WheelJoint_SetLimits(_id, lower, upper);
}

std::pair<float, float>
WheelJoint::limits() const
{
	float lower = b2WheelJoint_GetLowerLimit(_id);
	float upper = b2WheelJoint_GetUpperLimit(_id);
	return { lower, upper };
}

void
WheelJoint::enable_motor(bool flag)
{
	b2WheelJoint_EnableMotor(_id, flag);
}

bool
WheelJoint::is_motor_enabled() const
{
	return b2WheelJoint_IsMotorEnabled(_id);
}

void
WheelJoint::set_motor_speed(float speed)
{
	b2WheelJoint_SetMotorSpeed(_id, speed);
}

float
WheelJoint::motor_speed() const
{
	return b2WheelJoint_GetMotorSpeed(_id);
}

void
WheelJoint::set_max_motor_torque(float torque)
{
	b2WheelJoint_SetMaxMotorTorque(_id, torque);
}

float
WheelJoint::max_motor_torque() const
{
	return b2WheelJoint_GetMaxMotorTorque(_id);
}

float
WheelJoint::motor_torque() const
{
	return b2WheelJoint_GetMotorTorque(_id);
}

mrb_value
WheelJoint::wrap(const util::Reference<util::State> &state)
{
	auto self = util::Reference(this);
	return state->wrap<WheelJoint>(self);
}