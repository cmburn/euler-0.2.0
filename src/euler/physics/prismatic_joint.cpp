/* SPDX-License-Identifier: ISC */

#include "euler/physics/prismatic_joint.h"

#include <box2d/box2d.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/hash.h>

#include "euler/physics/body.h"
#include "euler/physics/joint.h"
#include "euler/physics/util.h"
#include "euler/physics/world.h"

using euler::physics::PrismaticJoint;

/**
 * @overload Euler::Physics::PrismaticJoint#spring_enabled=(flag)
 *   Enable or disable the prismatic joint's spring.
 *   @param flag [Boolean] true to enable the spring, false to disable it.
 *   @return [void]
 */
static mrb_value
prismatic_joint_enable_spring(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	mrb_bool flag;
	state->rb().get_args("b", &flag);
	joint->enable_spring(flag);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::PrismaticJoint#spring_enabled
 *   Check if the prismatic joint's spring is enabled.
 *   @return [Boolean] true if the spring is enabled, false otherwise.
 */
static mrb_value
prismatic_joint_is_spring_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const bool flag = joint->is_spring_enabled();
	return mrb_bool_value(flag);
}

/**
 * @overload Euler::Physics::PrismaticJoint#spring_hertz=(value)
 *   Set the spring stiffness in Hertz.
 *   @param value [Numeric] The spring stiffness in Hertz.
 *   @return [void]
 */
static mrb_value
prismatic_joint_set_spring_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	mrb_float hertz;
	state->rb().get_args("f", &hertz);
	joint->set_spring_hertz(static_cast<float>(hertz));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::PrismaticJoint#spring_hertz
 *   Get the spring stiffness in Hertz.
 *   @return [Numeric] The spring stiffness in Hertz.
 */
static mrb_value
prismatic_joint_spring_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const float hertz = joint->spring_hertz();
	return state->rb().float_value(hertz);
}

/**
 * @overload Euler::Physics::PrismaticJoint#spring_damping_ratio=(value)
 *   Set the spring damping ratio (non-dimensional).
 *   @param value [Numeric] The spring damping ratio.
 *   @return [void]
 */
static mrb_value
prismatic_joint_set_spring_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	mrb_float damping_ratio;
	state->rb().get_args("f", &damping_ratio);
	joint->set_spring_damping_ratio(static_cast<float>(damping_ratio));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::PrismaticJoint#spring_damping_ratio
 *   Get the spring damping ratio (non-dimensional).
 *   @return [Numeric] The spring damping ratio.
 */
static mrb_value
prismatic_joint_spring_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const float damping_ratio = joint->spring_damping_ratio();
	return state->rb().float_value(damping_ratio);
}

/**
 * @overload Euler::Physics::PrismaticJoint#target_translation=(value)
 *   Set the spring target translation, usually in meters.
 *   @param value [Numeric] The target translation in meters.
 *   @return [void]
 */
static mrb_value
prismatic_joint_set_target_translation(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	mrb_float translation;
	state->rb().get_args("f", &translation);
	joint->set_target_translation(static_cast<float>(translation));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::PrismaticJoint#target_translation
 *   Get the spring target translation, usually in meters.
 *   @return [Numeric] The target translation in meters.
 */
static mrb_value
prismatic_joint_target_translation(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const float translation = joint->target_translation();
	return state->rb().float_value(translation);
}

/**
 * @overload Euler::Physics::PrismaticJoint#limit_enabled=(flag)
 *   Enable or disable the prismatic joint's limit.
 *   @param flag [Boolean] true to enable the limit, false to disable it.
 *   @return [void]
 */
static mrb_value
prismatic_joint_enable_limit(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	mrb_bool flag;
	state->rb().get_args("b", &flag);
	joint->enable_limit(flag);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::PrismaticJoint#limit_enabled
 *   Check if the prismatic joint's limit is enabled.
 *   @return [Boolean] true if the limit is enabled, false otherwise.
 */
static mrb_value
prismatic_joint_is_limit_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const bool flag = joint->is_limit_enabled();
	return mrb_bool_value(flag);
}

/**
 * @overload Euler::Physics::PrismaticJoint#limits=(array)
 *   Set the prismatic joint limits.
 *   @param array [Array<Numeric>] An array containing two numeric values:
 *     the lower limit and the upper limit.
 *   @return [void]
 */
static mrb_value
prismatic_joint_set_limits(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	mrb_value arr = mrb_nil_value();
	state->rb().get_args("A", &arr);
	const b2Vec2 limits = euler::physics::value_to_b2_vec(mrb, arr);
	joint->set_limits(limits.x, limits.y);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::PrismaticJoint#limits
 *   Get the prismatic joint limits.
 *   @return [Array<Numeric>] An array containing two numeric values:
 *     the lower limit and the upper limit.
 */
static mrb_value
prismatic_joint_limits(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const auto [lower_limit, upper_limit] = joint->limits();
	const mrb_value out = state->rb().ary_new_capa(2);
	state->rb().ary_push(out, state->rb().float_value(lower_limit));
	state->rb().ary_push(out, state->rb().float_value(upper_limit));
	return out;
}

/**
 * @overload Euler::Physics::PrismaticJoint#motor_enabled=(flag)
 *   Enable or disable the prismatic joint's motor.
 *   @param flag [Boolean] true to enable the motor, false to disable it.
 *   @return [void]
 */
static mrb_value
prismatic_joint_enable_motor(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	mrb_bool flag;
	state->rb().get_args("b", &flag);
	joint->enable_motor(flag);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::PrismaticJoint#motor_enabled
 *   Check if the prismatic joint's motor is enabled.
 *   @return [Boolean] true if the motor is enabled, false otherwise.
 */
static mrb_value
prismatic_joint_is_motor_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const bool flag = joint->is_motor_enabled();
	return mrb_bool_value(flag);
}

/**
 * @overload Euler::Physics::PrismaticJoint#motor_speed=(value)
 *   Set the prismatic joint motor speed, usually in meters per second.
 *   @param value [Numeric] The motor speed in meters per second.
 *   @return [void]
 */
static mrb_value
prismatic_joint_set_motor_speed(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	mrb_float motor_speed;
	state->rb().get_args("f", &motor_speed);
	joint->set_motor_speed(static_cast<float>(motor_speed));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::PrismaticJoint#motor_speed
 *   Get the prismatic joint motor speed, usually in meters per second.
 *   @return [Numeric] The motor speed in meters per second.
 */
static mrb_value
prismatic_joint_motor_speed(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const float motor_speed = joint->motor_speed();
	return state->rb().float_value(motor_speed);
}

/**
 * @overload Euler::Physics::PrismaticJoint#max_motor_force=(value)
 *   Set the prismatic joint maximum motor force, usually in newtons.
 *   @param value [Numeric] The maximum motor force in newtons.
 *   @return [void]
 */
static mrb_value
prismatic_joint_set_max_motor_force(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	mrb_float force;
	state->rb().get_args("f", &force);
	joint->set_max_motor_force(static_cast<float>(force));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::PrismaticJoint#max_motor_force
 *   Get the prismatic joint maximum motor force, usually in newtons.
 *   @return [Numeric] The maximum motor force in newtons.
 */
static mrb_value
prismatic_joint_max_motor_force(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const float force = joint->max_motor_force();
	return state->rb().float_value(force);
}

/**
 * @overload Euler::Physics::PrismaticJoint#motor_force
 *   Get the prismatic joint current motor force, usually in newtons.
 *   @return [Numeric] The current motor force in newtons.
 */
static mrb_value
prismatic_joint_motor_force(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const float force = joint->motor_force();
	return state->rb().float_value(force);
}

/**
 * @overload Euler::Physics::PrismaticJoint#translation
 *   Get the current joint translation, usually in meters.
 *   @return [Numeric] The current joint translation in meters.
 */
static mrb_value
prismatic_joint_translation(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const float translation = joint->translation();
	return state->rb().float_value(translation);
}

/**
 * @overload Euler::Physics::PrismaticJoint#speed
 *   Get the current joint translation speed, usually in meters per second.
 *   @return [Numeric] The current joint translation speed in meters per second.
 */
static mrb_value
prismatic_joint_speed(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<PrismaticJoint>(self);
	const float speed = joint->speed();
	return state->rb().float_value(speed);
}

static RClass *
prismatic_joint_init(mrb_state *mrb, RClass *mod, RClass *super)
{
	const auto state = euler::util::State::get(mrb);
	RClass *joint
	    = state->rb().define_class_under(mod, "PrismaticJoint", super);
	state->rb().define_method(joint,
	    "spring_enabled=", prismatic_joint_enable_spring, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "spring_enabled",
	    prismatic_joint_is_spring_enabled, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "spring_hertz=", prismatic_joint_set_spring_hertz, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "spring_hertz",
	    prismatic_joint_spring_hertz, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "spring_damping_ratio=", prismatic_joint_set_spring_damping_ratio,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "spring_damping_ratio",
	    prismatic_joint_spring_damping_ratio, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "target_translation=", prismatic_joint_set_target_translation,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "target_translation",
	    prismatic_joint_target_translation, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "limit_enabled=", prismatic_joint_enable_limit, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "limit_enabled",
	    prismatic_joint_is_limit_enabled, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "limits=", prismatic_joint_set_limits, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "limits", prismatic_joint_limits,
	    MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "motor_enabled=", prismatic_joint_enable_motor, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "motor_enabled",
	    prismatic_joint_is_motor_enabled, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "motor_speed=", prismatic_joint_set_motor_speed, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "motor_speed",
	    prismatic_joint_motor_speed, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "max_motor_force=", prismatic_joint_set_max_motor_force,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "max_motor_force",
	    prismatic_joint_max_motor_force, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "motor_force",
	    prismatic_joint_motor_force, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "translation",
	    prismatic_joint_translation, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "speed", prismatic_joint_speed,
	    MRB_ARGS_REQ(0));
	return joint;
}

RClass *
PrismaticJoint::init(const euler::util::Reference<euler::util::State> &state,
    RClass *mod, RClass *super)
{
	return prismatic_joint_init(state->rb().mrb(), mod, super);
}

void
PrismaticJoint::enable_spring(bool flag)
{
	b2PrismaticJoint_EnableSpring(_id, flag);
}
bool
PrismaticJoint::is_spring_enabled()
{
	return b2PrismaticJoint_IsSpringEnabled(_id);
}
void
PrismaticJoint::set_spring_hertz(float hertz)
{
	b2PrismaticJoint_SetSpringHertz(_id, hertz);
}
float
PrismaticJoint::spring_hertz()
{
	return b2PrismaticJoint_GetSpringHertz(_id);
}
void
PrismaticJoint::set_spring_damping_ratio(float ratio)
{
	b2PrismaticJoint_SetSpringDampingRatio(_id, ratio);
}
float
PrismaticJoint::spring_damping_ratio()
{
	return b2PrismaticJoint_GetSpringDampingRatio(_id);
}
void
PrismaticJoint::set_target_translation(float translation)
{
	b2PrismaticJoint_SetTargetTranslation(_id, translation);
}
float
PrismaticJoint::target_translation()
{
	return b2PrismaticJoint_GetTargetTranslation(_id);
}
void
PrismaticJoint::enable_limit(bool flag)
{
	b2PrismaticJoint_EnableLimit(_id, flag);
}
bool
PrismaticJoint::is_limit_enabled()
{
	return b2PrismaticJoint_IsLimitEnabled(_id);
}
void
PrismaticJoint::set_limits(float lower, float upper)
{
	b2PrismaticJoint_SetLimits(_id, lower, upper);
}
std::pair<float, float>
PrismaticJoint::limits()
{
	float lower = b2PrismaticJoint_GetLowerLimit(_id);
	float upper = b2PrismaticJoint_GetUpperLimit(_id);
	return { lower, upper };
}
void
PrismaticJoint::enable_motor(bool flag)
{
	b2PrismaticJoint_EnableMotor(_id, flag);
}
bool
PrismaticJoint::is_motor_enabled()
{
	return b2PrismaticJoint_IsMotorEnabled(_id);
}
void
PrismaticJoint::set_motor_speed(float speed)
{
	b2PrismaticJoint_SetMotorSpeed(_id, speed);
}
float
PrismaticJoint::motor_speed()
{
	return b2PrismaticJoint_GetMotorSpeed(_id);
}
void
PrismaticJoint::set_max_motor_force(float force)
{
	b2PrismaticJoint_SetMaxMotorForce(_id, force);
}
float
PrismaticJoint::max_motor_force()
{
	return b2PrismaticJoint_GetMaxMotorForce(_id);
}
float
PrismaticJoint::motor_force()
{
	return b2PrismaticJoint_GetMotorForce(_id);
}
float
PrismaticJoint::translation()
{
	return b2PrismaticJoint_GetTranslation(_id);
}
float
PrismaticJoint::speed()
{
	return b2PrismaticJoint_GetSpeed(_id);
}
mrb_value
PrismaticJoint::wrap(const util::Reference<util::State> &state)
{
	auto self = util::Reference(this);
	return state->wrap<PrismaticJoint>(self);
}