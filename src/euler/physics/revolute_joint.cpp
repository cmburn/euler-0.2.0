/* SPDX-License-Identifier: ISC */

#include "euler/physics/revolute_joint.h"

/* SPDX-License-Identifier: ISC */

#include "euler/physics/revolute_joint.h"

#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/hash.h>

#include "euler/physics/body.h"
#include "euler/physics/joint.h"
#include "euler/physics/util.h"
#include "euler/physics/world.h"

using euler::physics::RevoluteJoint;

/**
 * @overload Euler::Physics::RevoluteJoint#spring_enabled=(flag)
 *   Enable or disable the revolute joint's spring.
 *   @param flag [Boolean] true to enable the spring, false to disable it.
 *   @return [void]
 */
static mrb_value
revolute_joint_enable_spring(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	mrb_bool flag;
	state->rb().get_args("b", &flag);
	joint->enable_spring(flag);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::RevoluteJoint#spring_enabled
 *   Check if the revolute joint's spring is enabled.
 *   @return [Boolean] true if the spring is enabled, false otherwise.
 */
static mrb_value
revolute_joint_is_spring_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	const bool flag = joint->is_spring_enabled();
	return mrb_bool_value(flag);
}

/**
 * @overload Euler::Physics::RevoluteJoint#spring_hertz=(value)
 *   Set the spring stiffness in Hertz.
 *   @param value [Numeric] The spring stiffness in Hertz.
 *   @return [void]
 */
static mrb_value
revolute_joint_set_spring_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	mrb_float hertz;
	state->rb().get_args("f", &hertz);
	joint->set_spring_hertz(static_cast<float>(hertz));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::RevoluteJoint#spring_hertz
 *   Get the spring stiffness in Hertz.
 *   @return [Numeric] The spring stiffness in Hertz.
 */
static mrb_value
revolute_joint_spring_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	const float hertz = joint->spring_hertz();
	return state->rb().float_value(hertz);
}

/**
 * @overload Euler::Physics::RevoluteJoint#spring_damping_ratio=(value)
 *   Set the spring damping ratio, non-dimensional.
 *   @param value [Numeric] The spring damping ratio.
 *   @return [void]
 */
static mrb_value
revolute_joint_set_spring_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	mrb_float damping_ratio;
	state->rb().get_args("f", &damping_ratio);
	joint->set_spring_damping_ratio((float)damping_ratio);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::RevoluteJoint#spring_damping_ratio
 *   Get the spring damping ratio, non-dimensional.
 *   @return [Numeric] The spring damping ratio.
 */
static mrb_value
revolute_joint_spring_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	const float damping_ratio = joint->spring_damping_ratio();
	return state->rb().float_value(damping_ratio);
}

/**
 * @overload Euler::Physics::RevoluteJoint#target_angle=(value)
 *   Set the revolute joint spring target angle, in radians.
 *   @param value [Numeric] The target angle in radians.
 *   @return [void]
 */
static mrb_value
revolute_joint_set_target_angle(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	mrb_float angle;
	state->rb().get_args("f", &angle);
	joint->set_target_angle((float)angle);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::RevoluteJoint#target_angle
 *   Get the revolute joint spring target angle, in radians.
 *   @return [Numeric] The target angle in radians.
 */
static mrb_value
revolute_joint_target_angle(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	const float angle = joint->target_angle();
	return state->rb().float_value(angle);
}

/**
 * @overload Euler::Physics::RevoluteJoint#angle
 *   Get the current joint angle in radians relative to the reference angle.
 *   @return [Numeric] The current joint angle in radians.
 */
static mrb_value
revolute_joint_angle(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	const float angle = joint->angle();
	return state->rb().float_value(angle);
}

/**
 * @overload Euler::Physics::RevoluteJoint#limit_enabled=(flag)
 *   Enable or disable the revolute joint's limit.
 *   @param flag [Boolean] true to enable the limit, false to disable it.
 *   @return [void]
 */
static mrb_value
revolute_joint_enable_limit(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	mrb_bool flag;
	state->rb().get_args("b", &flag);
	joint->enable_limit(flag);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::RevoluteJoint#limit_enabled
 *   Check if the revolute joint's limit is enabled.
 *   @return [Boolean] true if the limit is enabled, false otherwise.
 */
static mrb_value
revolute_joint_is_limit_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	const bool flag = joint->is_limit_enabled();
	return mrb_bool_value(flag);
}

/**
 * @overload Euler::Physics::RevoluteJoint#limits=(array)
 *   Set the revolute joint limits.
 *   @param array [Array<Numeric>] An array containing the lower and upper
 *          limits in radians.
 *   @return [void]
 */
static mrb_value
revolute_joint_set_limits(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	mrb_value arr = mrb_nil_value();
	state->rb().get_args("A", &arr);
	const b2Vec2 limits = euler::physics::value_to_b2_vec(mrb, arr);
	joint->set_limits(limits.x, limits.y);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::RevoluteJoint#limits
 *   Get the revolute joint limits.
 *   @return [Array<Numeric>] An array containing the lower and upper limits
 *          in radians.
 */
static mrb_value
revolute_joint_limits(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	const auto [lower_limit, upper_limit] = joint->limits();
	const mrb_value out = state->rb().ary_new_capa(2);
	state->rb().ary_push(out, state->rb().float_value(lower_limit));
	state->rb().ary_push(out, state->rb().float_value(upper_limit));
	return out;
}

/**
 * @overload Euler::Physics::RevoluteJoint#motor_enabled=(flag)
 *   Enable or disable the revolute joint's motor.
 *   @param flag [Boolean] true to enable the motor, false to disable it.
 *   @return [void]
 */
static mrb_value
revolute_joint_enable_motor(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	mrb_bool flag;
	state->rb().get_args("b", &flag);
	joint->enable_motor(flag);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::RevoluteJoint#motor_enabled
 *   Check if the revolute joint's motor is enabled.
 *   @return [Boolean] true if the motor is enabled, false otherwise.
 */
static mrb_value
revolute_joint_is_motor_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	const bool flag = joint->is_motor_enabled();
	return mrb_bool_value(flag);
}

/**
 * @overload Euler::Physics::RevoluteJoint#motor_speed=(value)
 *   Set the revolute joint motor speed, usually in radians per second.
 *   @param value [Numeric] The motor speed in radians per second.
 *   @return [void]
 */
static mrb_value
revolute_joint_set_motor_speed(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	mrb_float motor_speed;
	state->rb().get_args("f", &motor_speed);
	joint->set_motor_speed((float)motor_speed);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::RevoluteJoint#motor_speed
 *   Get the revolute joint motor speed, usually in radians per second.
 *   @return [Numeric] The motor speed in radians per second.
 */
static mrb_value
revolute_joint_motor_speed(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	const float motor_speed = joint->motor_speed();
	return state->rb().float_value(motor_speed);
}

/**
 * @overload Euler::Physics::RevoluteJoint#motor_torque
 *   Get the revolute joint current motor torque, usually in newton-meters.
 *   @return [Numeric] The current motor torque in newton-meters.
 */
static mrb_value
revolute_joint_motor_torque(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	const float torque = joint->motor_torque();
	return state->rb().float_value(torque);
}

/**
 * @overload Euler::Physics::RevoluteJoint#max_motor_torque=(value)
 *   Set the revolute joint maximum motor torque, usually in newton-meters.
 *   @param value [Numeric] The maximum motor torque in newton-meters.
 *   @return [void]
 */
static mrb_value
revolute_joint_set_max_motor_torque(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	mrb_float max_torque;
	state->rb().get_args("f", &max_torque);
	joint->set_max_motor_torque((float)max_torque);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::RevoluteJoint#max_motor_torque
 *   Get the revolute joint maximum motor torque, usually in newton-meters.
 *   @return [Numeric] The maximum motor torque in newton-meters.
 */
static mrb_value
revolute_joint_max_motor_torque(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<RevoluteJoint>(self);
	const float max_torque = joint->max_motor_torque();
	return state->rb().float_value(max_torque);
}

RClass *
box2d_revolute_joint_init(mrb_state *mrb, RClass *mod, RClass *super)
{
	const auto state = euler::util::State::get(mrb);
	RClass *joint
	    = state->rb().define_class_under(mod, "RevoluteJoint", super);
	state->rb().define_method(joint,
	    "spring_enabled=", revolute_joint_enable_spring, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "spring_enabled",
	    revolute_joint_is_spring_enabled, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "spring_hertz=", revolute_joint_set_spring_hertz, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "spring_hertz",
	    revolute_joint_spring_hertz, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "spring_damping_ratio=", revolute_joint_set_spring_damping_ratio,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "spring_damping_ratio",
	    revolute_joint_spring_damping_ratio, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "target_angle=", revolute_joint_set_target_angle, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "target_angle",
	    revolute_joint_target_angle, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "angle", revolute_joint_angle,
	    MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "limit_enabled=", revolute_joint_enable_limit, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "limit_enabled",
	    revolute_joint_is_limit_enabled, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "limits=", revolute_joint_set_limits,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "limits", revolute_joint_limits,
	    MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "motor_enabled=", revolute_joint_enable_motor, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "motor_enabled",
	    revolute_joint_is_motor_enabled, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "motor_speed=", revolute_joint_set_motor_speed, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "motor_speed",
	    revolute_joint_motor_speed, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "motor_torque",
	    revolute_joint_motor_torque, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "max_motor_torque=", revolute_joint_set_max_motor_torque,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "max_motor_torque",
	    revolute_joint_max_motor_torque, MRB_ARGS_REQ(0));
	return joint;
}
RClass *
RevoluteJoint::init(const euler::util::Reference<euler::util::State> &state,
    RClass *mod, RClass *super)
{
	return box2d_revolute_joint_init(state->rb().mrb(), mod, super);
}
void
RevoluteJoint::enable_spring(bool flag)
{
	b2RevoluteJoint_EnableSpring(_id, flag);
}
bool
RevoluteJoint::is_spring_enabled() const
{
	return b2RevoluteJoint_IsSpringEnabled(_id);
}
void
RevoluteJoint::set_spring_hertz(float hertz)
{
	b2RevoluteJoint_SetSpringHertz(_id, hertz);
}
float
RevoluteJoint::spring_hertz() const
{
	return b2RevoluteJoint_GetSpringHertz(_id);
}
void
RevoluteJoint::set_spring_damping_ratio(float ratio)
{
	b2RevoluteJoint_SetSpringDampingRatio(_id, ratio);
}
float
RevoluteJoint::spring_damping_ratio() const
{
	return b2RevoluteJoint_GetSpringDampingRatio(_id);
}
void
RevoluteJoint::set_target_angle(float angle)
{
	b2RevoluteJoint_SetTargetAngle(_id, angle);
}
float
RevoluteJoint::target_angle() const
{
	return b2RevoluteJoint_GetTargetAngle(_id);
}
float
RevoluteJoint::angle() const
{
	return b2RevoluteJoint_GetAngle(_id);
}
void
RevoluteJoint::enable_limit(bool flag)
{
	b2RevoluteJoint_EnableLimit(_id, flag);
}
bool
RevoluteJoint::is_limit_enabled() const
{
	return b2RevoluteJoint_IsLimitEnabled(_id);
}
void
RevoluteJoint::set_limits(float lower, float upper)
{
	b2RevoluteJoint_SetLimits(_id, lower, upper);
}
std::pair<float, float>
RevoluteJoint::limits() const
{
	return {
		b2RevoluteJoint_GetLowerLimit(_id),
		b2RevoluteJoint_GetUpperLimit(_id),
	};
}
void
RevoluteJoint::enable_motor(bool flag)
{
	b2RevoluteJoint_EnableMotor(_id, flag);
}
bool
RevoluteJoint::is_motor_enabled() const
{
	return b2RevoluteJoint_IsMotorEnabled(_id);
}
void
RevoluteJoint::set_motor_speed(float speed)
{
	b2RevoluteJoint_SetMotorSpeed(_id, speed);
}
float
RevoluteJoint::motor_speed() const
{
	return b2RevoluteJoint_GetMotorSpeed(_id);
}
float
RevoluteJoint::motor_torque() const
{
	return b2RevoluteJoint_GetMotorTorque(_id);
}
void
RevoluteJoint::set_max_motor_torque(float torque)
{
	b2RevoluteJoint_SetMaxMotorTorque(_id, torque);
}
float
RevoluteJoint::max_motor_torque() const
{
	return b2RevoluteJoint_GetMaxMotorTorque(_id);
}
mrb_value
RevoluteJoint::wrap(const util::Reference<util::State> &state)
{
	auto self = util::Reference(this);
	return state->wrap<RevoluteJoint>(self);
}