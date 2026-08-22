/* SPDX-License-Identifier: ISC */

#include "euler/physics/motor_joint.h"

/* SPDX-License-Identifier: ISC */

#include "euler/physics/motor_joint.h"

#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/hash.h>

#include "euler/physics/body.h"
#include "euler/physics/joint.h"
#include "euler/physics/util.h"
#include "euler/physics/world.h"

using euler::physics::MotorJoint;

/**
 * @overload Euler::Physics::MotorJoint#linear_velocity=(value)
 *   Set the target linear velocity of the motor joint.
 *   @param value [Array<Numeric>] The target linear velocity as [x, y].
 *   @return [void]
 */
static mrb_value
motor_joint_set_linear_velocity(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	mrb_value value;
	state->rb().get_args("o", &value);
	const b2Vec2 linear_velocity
	    = euler::physics::value_to_b2_vec(mrb, value);
	joint->set_linear_velocity(linear_velocity);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::MotorJoint#linear_velocity
 *   Get the target linear velocity of the motor joint.
 *   @return [Array<Numeric>] The target linear velocity as [x, y].
 */
static mrb_value
motor_joint_linear_velocity(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	const b2Vec2 linear_velocity = joint->linear_velocity();
	return euler::physics::b2_vec_to_value(mrb, linear_velocity);
}

/**
 * @overload Euler::Physics::MotorJoint#angular_velocity=(value)
 *   Set the target angular velocity of the motor joint.
 *   @param value [Numeric] The target angular velocity in radians per second.
 *   @return [void]
 */
static mrb_value
motor_joint_set_angular_velocity(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	mrb_float angular_velocity;
	state->rb().get_args("f", &angular_velocity);
	joint->set_angular_velocity(static_cast<float>(angular_velocity));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::MotorJoint#angular_velocity
 *   Get the target angular velocity of the motor joint.
 *   @return [Numeric] The target angular velocity in radians per second.
 */
static mrb_value
motor_joint_angular_velocity(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	const float angular_velocity = joint->angular_velocity();
	return state->rb().float_value(angular_velocity);
}

/**
 * @overload Euler::Physics::MotorJoint#max_velocity_force=(value)
 *   Set the maximum force the motor joint can apply to achieve the target
 *   linear velocity.
 *   @param value [Numeric] The maximum force in newtons.
 *   @return [void]
 */
static mrb_value
motor_joint_set_max_velocity_force(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	mrb_float max_force;
	state->rb().get_args("f", &max_force);
	joint->set_max_velocity_force(static_cast<float>(max_force));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::MotorJoint#max_velocity_force
 *   Get the maximum force the motor joint can apply to achieve the target
 *   linear velocity.
 *   @return [Numeric] The maximum force in newtons.
 */
static mrb_value
motor_joint_max_velocity_force(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	const float max_force = joint->max_velocity_force();
	return state->rb().float_value(max_force);
}

/**
 * @overload Euler::Physics::MotorJoint#max_velocity_torque=(value)
 *   Set the maximum torque the motor joint can apply to achieve the target
 *   angular velocity.
 *   @param value [Numeric] The maximum torque in newton-meters.
 *   @return [void]
 */
static mrb_value
motor_joint_set_max_velocity_torque(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	mrb_float max_torque;
	state->rb().get_args("f", &max_torque);
	joint->set_max_velocity_torque(static_cast<float>(max_torque));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::MotorJoint#max_velocity_torque
 *   Get the maximum torque the motor joint can apply to achieve the target
 *   angular velocity.
 *   @return [Numeric] The maximum torque in newton-meters
 */
static mrb_value
motor_joint_max_velocity_torque(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	const float max_torque = joint->max_velocity_torque();
	return state->rb().float_value(max_torque);
}

/**
 * @overload Euler::Physics::MotorJoint#linear_hertz=(value)
 *   Set the linear spring stiffness in Hertz.
 *   @param value [Numeric] The linear spring stiffness in Hertz.
 *   @return [void]
 */
static mrb_value
motor_joint_set_linear_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	mrb_float hertz;
	state->rb().get_args("f", &hertz);
	joint->set_linear_hertz(static_cast<float>(hertz));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::MotorJoint#linear_hertz
 *   Get the linear spring stiffness in Hertz.
 *   @return [Numeric] The linear spring stiffness in Hertz.
 */
static mrb_value
motor_joint_linear_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	const float hertz = joint->linear_hertz();
	return state->rb().float_value(hertz);
}

/**
 * @overload Euler::Physics::MotorJoint#linear_damping_ratio=(value)
 *   Set the linear spring damping ratio (non-dimensional).
 *   @param value [Numeric] The linear spring damping ratio.
 *   @return [void]
 */
static mrb_value
motor_joint_set_linear_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	mrb_float damping_ratio;
	state->rb().get_args("f", &damping_ratio);
	joint->set_linear_damping_ratio(static_cast<float>(damping_ratio));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::MotorJoint#linear_damping_ratio
 *   Get the linear spring damping ratio (non-dimensional).
 *   @return [Numeric] The linear spring damping ratio.
 */
static mrb_value
motor_joint_linear_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	const float damping_ratio = joint->linear_damping_ratio();
	return state->rb().float_value(damping_ratio);
}

/**
 * @overload Euler::Physics::MotorJoint#angular_hertz=(value)
 *   Set the angular spring stiffness in Hertz.
 *   @param value [Numeric] The angular spring stiffness in Hertz.
 *   @return [void]
 */
static mrb_value
motor_joint_set_angular_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	mrb_float hertz;
	state->rb().get_args("f", &hertz);
	joint->set_angular_hertz(static_cast<float>(hertz));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::MotorJoint#angular_hertz
 *   Get the angular spring stiffness in Hertz.
 *   @return [Numeric] The angular spring stiffness in Hertz.
 */
static mrb_value
motor_joint_angular_hertz(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	const float hertz = joint->angular_hertz();
	return state->rb().float_value(hertz);
}

/**
 * @overload Euler::Physics::MotorJoint#angular_damping_ratio=(value)
 *   Set the angular spring damping ratio (non-dimensional).
 *   @param value [Numeric] The angular spring damping ratio.
 *   @return [void]
 */
static mrb_value
motor_joint_set_angular_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	mrb_float damping_ratio;
	state->rb().get_args("f", &damping_ratio);
	joint->set_angular_damping_ratio(static_cast<float>(damping_ratio));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::MotorJoint#angular_damping_ratio
 *   Get the angular spring damping ratio (non-dimensional).
 *   @return [Numeric] The angular spring damping ratio.
 */
static mrb_value
motor_joint_angular_damping_ratio(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	const float damping_ratio = joint->angular_damping_ratio();
	return state->rb().float_value(damping_ratio);
}

/**
 * @overload Euler::Physics::MotorJoint#max_spring_force=(value)
 *   Set the maximum spring force the motor joint can apply.
 *   @param value [Numeric] The maximum spring force in newtons.
 *   @return [void]
 */
static mrb_value
motor_joint_set_max_spring_force(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	mrb_float max_force;
	state->rb().get_args("f", &max_force);
	joint->set_max_spring_force(static_cast<float>(max_force));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::MotorJoint#max_spring_force
 *   Get the maximum spring force the motor joint can apply.
 *   @return [Numeric] The maximum spring force in newtons.
 */
static mrb_value
motor_joint_max_spring_force(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	const float max_force = joint->max_spring_force();
	return state->rb().float_value(max_force);
}

/**
 * @overload Euler::Physics::MotorJoint#max_spring_torque=(value)
 *   Set the maximum spring torque the motor joint can apply.
 *   @param value [Numeric] The maximum spring torque in newton-meters.
 *   @return [void]
 */
static mrb_value
motor_joint_set_max_spring_torque(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	mrb_float max_torque;
	state->rb().get_args("f", &max_torque);
	joint->set_max_spring_torque(static_cast<float>(max_torque));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::MotorJoint#max_spring_torque
 *   Get the maximum spring torque the motor joint can apply.
 *   @return [Numeric] The maximum spring torque in newton-meters.
 */
static mrb_value
motor_joint_max_spring_torque(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<MotorJoint>(self);
	const float max_torque = joint->max_spring_torque();
	return state->rb().float_value(max_torque);
}

RClass *
MotorJoint::init(const euler::util::Reference<euler::util::State> &state,
    RClass *mod, RClass *super)
{
	RClass *joint
	    = state->rb().define_class_under(mod, "MotorJoint", super);
	state->rb().define_method(joint,
	    "linear_velocity=", motor_joint_set_linear_velocity,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "linear_velocity",
	    motor_joint_linear_velocity, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "angular_velocity=", motor_joint_set_angular_velocity,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "angular_velocity",
	    motor_joint_angular_velocity, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "max_velocity_force=", motor_joint_set_max_velocity_force,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "max_velocity_force",
	    motor_joint_max_velocity_force, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "max_velocity_torque=", motor_joint_set_max_velocity_torque,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "max_velocity_torque",
	    motor_joint_max_velocity_torque, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "linear_hertz=", motor_joint_set_linear_hertz, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "linear_hertz",
	    motor_joint_linear_hertz, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "linear_damping_ratio=", motor_joint_set_linear_damping_ratio,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "linear_damping_ratio",
	    motor_joint_linear_damping_ratio, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "angular_hertz=", motor_joint_set_angular_hertz, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "angular_hertz",
	    motor_joint_angular_hertz, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "angular_damping_ratio=", motor_joint_set_angular_damping_ratio,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "angular_damping_ratio",
	    motor_joint_angular_damping_ratio, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "max_spring_force=", motor_joint_set_max_spring_force,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "max_spring_force",
	    motor_joint_max_spring_force, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "max_spring_torque=", motor_joint_set_max_spring_torque,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "max_spring_torque",
	    motor_joint_max_spring_torque, MRB_ARGS_REQ(0));
	return joint;
}

b2Vec2
MotorJoint::linear_velocity() const
{
	return b2MotorJoint_GetLinearVelocity(_id);
}

void
MotorJoint::set_linear_velocity(b2Vec2 velocity)
{
	b2MotorJoint_SetLinearVelocity(_id, velocity);
}

float
MotorJoint::angular_velocity() const
{
	return b2MotorJoint_GetAngularVelocity(_id);
}

void
MotorJoint::set_angular_velocity(float velocity)
{
	b2MotorJoint_SetAngularVelocity(_id, velocity);
}

float
MotorJoint::max_velocity_force() const
{
	return b2MotorJoint_GetMaxVelocityForce(_id);
}

void
MotorJoint::set_max_velocity_force(float force)
{
	b2MotorJoint_SetMaxVelocityForce(_id, force);
}

float
MotorJoint::max_velocity_torque() const
{
	return b2MotorJoint_GetMaxVelocityTorque(_id);
}

void
MotorJoint::set_max_velocity_torque(float torque)
{
	b2MotorJoint_SetMaxVelocityTorque(_id, torque);
}

float
MotorJoint::linear_hertz() const
{
	return b2MotorJoint_GetLinearHertz(_id);
}

void
MotorJoint::set_linear_hertz(float hertz)
{
	b2MotorJoint_SetLinearHertz(_id, hertz);
}

float
MotorJoint::linear_damping_ratio() const
{
	return b2MotorJoint_GetLinearDampingRatio(_id);
}

void
MotorJoint::set_linear_damping_ratio(float ratio)
{
	b2MotorJoint_SetLinearDampingRatio(_id, ratio);
}

float
MotorJoint::angular_hertz() const
{
	return b2MotorJoint_GetAngularHertz(_id);
}

void
MotorJoint::set_angular_hertz(float hertz)
{
	b2MotorJoint_SetAngularHertz(_id, hertz);
}

float
MotorJoint::angular_damping_ratio() const
{
	return b2MotorJoint_GetAngularDampingRatio(_id);
}

void
MotorJoint::set_angular_damping_ratio(float ratio)
{
	b2MotorJoint_SetAngularDampingRatio(_id, ratio);
}

float
MotorJoint::max_spring_force() const
{
	return b2MotorJoint_GetMaxSpringForce(_id);
}

void
MotorJoint::set_max_spring_force(float force)
{
	b2MotorJoint_SetMaxSpringForce(_id, force);
}

float
MotorJoint::max_spring_torque() const
{
	return b2MotorJoint_GetMaxSpringTorque(_id);
}

void
MotorJoint::set_max_spring_torque(float torque)
{
	b2MotorJoint_SetMaxSpringTorque(_id, torque);
}

mrb_value
MotorJoint::wrap(const util::Reference<util::State> &state)
{
	auto self = util::Reference(this);
	return state->wrap<MotorJoint>(self);
}
