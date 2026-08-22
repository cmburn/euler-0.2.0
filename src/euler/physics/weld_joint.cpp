/* SPDX-License-Identifier: ISC */

#include "euler/physics/weld_joint.h"

/* SPDX-License-Identifier: ISC */

#include "euler/physics/weld_joint.h"

#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/hash.h>

#include "euler/physics/body.h"
#include "euler/physics/joint.h"
#include "euler/physics/util.h"
#include "euler/physics/world.h"

using euler::physics::WeldJoint;

/**
 * @overload Euler::Physics::WeldJoint#linear_hertz=(value)
 *   Set the linear spring frequency in Hertz.
 *   @param value [Numeric] The linear spring frequency in Hertz.
 *   @return [void]
 */
static mrb_value
weld_joint_set_linear_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WeldJoint>(self);
	mrb_float hertz;
	state->rb().get_args("f", &hertz);
	joint->set_linear_hertz(static_cast<float>(hertz));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::WeldJoint#linear_hertz
 *   Get the linear spring frequency in Hertz.
 *   @return [Numeric] The linear spring frequency in Hertz.
 */
static mrb_value
weld_joint_linear_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WeldJoint>(self);
	const float hertz = joint->linear_hertz();
	return state->rb().float_value(hertz);
}

static mrb_value
weld_joint_set_linear_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WeldJoint>(self);
	mrb_float damping_ratio;
	state->rb().get_args("f", &damping_ratio);
	joint->set_linear_damping_ratio(static_cast<float>(damping_ratio));
	return mrb_nil_value();
}

static mrb_value
weld_joint_linear_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WeldJoint>(self);
	const float damping_ratio = joint->linear_damping_ratio();
	return state->rb().float_value(damping_ratio);
}

static mrb_value
weld_joint_set_angular_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WeldJoint>(self);
	mrb_float hertz;
	state->rb().get_args("f", &hertz);
	joint->set_angular_hertz(static_cast<float>(hertz));
	return mrb_nil_value();
}

static mrb_value
weld_joint_angular_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WeldJoint>(self);
	const float hertz = joint->angular_hertz();
	return state->rb().float_value(hertz);
}

static mrb_value
weld_joint_set_angular_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WeldJoint>(self);
	mrb_float damping_ratio;
	state->rb().get_args("f", &damping_ratio);
	joint->set_angular_damping_ratio(static_cast<float>(damping_ratio));
	return mrb_nil_value();
}

static mrb_value
weld_joint_angular_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<WeldJoint>(self);
	const float damping_ratio = joint->angular_damping_ratio();
	return state->rb().float_value(damping_ratio);
}

static RClass *
weld_joint_init(mrb_state *mrb, RClass *mod, RClass *super)
{
	const auto state = euler::util::State::get(mrb);
	RClass *joint
	    = state->rb().define_class_under(mod, "WeldJoint", super);
	state->rb().define_method(joint,
	    "linear_hertz=", weld_joint_set_linear_hertz, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "linear_hertz",
	    weld_joint_linear_hertz, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "linear_damping_ratio=", weld_joint_set_linear_damping_ratio,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "linear_damping_ratio",
	    weld_joint_linear_damping_ratio, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "angular_hertz=", weld_joint_set_angular_hertz, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "angular_hertz",
	    weld_joint_angular_hertz, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "angular_damping_ratio=", weld_joint_set_angular_damping_ratio,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "angular_damping_ratio",
	    weld_joint_angular_damping_ratio, MRB_ARGS_REQ(0));
	return joint;
}

RClass *
WeldJoint::init(const util::Reference<util::State> &state, RClass *mod,
    RClass *super)
{
	return weld_joint_init(state->rb().mrb(), mod, super);
}

void
WeldJoint::set_linear_hertz(float hertz)
{
	b2WeldJoint_SetLinearHertz(_id, hertz);
}

float
WeldJoint::linear_hertz() const
{
	return b2WeldJoint_GetLinearHertz(_id);
}

void
WeldJoint::set_linear_damping_ratio(float ratio)
{
	b2WeldJoint_SetLinearDampingRatio(_id, ratio);
}

float
WeldJoint::linear_damping_ratio() const
{
	return b2WeldJoint_GetLinearDampingRatio(_id);
}

void
WeldJoint::set_angular_hertz(float hertz)
{
	b2WeldJoint_SetAngularHertz(_id, hertz);
}

float
WeldJoint::angular_hertz() const
{
	return b2WeldJoint_GetAngularHertz(_id);
}

void
WeldJoint::set_angular_damping_ratio(float ratio)
{
	b2WeldJoint_SetAngularDampingRatio(_id, ratio);
}

float
WeldJoint::angular_damping_ratio() const
{
	return b2WeldJoint_GetAngularDampingRatio(_id);
}
mrb_value
WeldJoint::wrap(const util::Reference<util::State> &state)
{
	auto self = util::Reference(this);
	return state->wrap<WeldJoint>(self);
}