/* SPDX-License-Identifier: ISC */

#include "euler/physics/joint.h"

/* SPDX-License-Identifier: ISC */

#include <utility>

#include "euler/physics/joint.h"

#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/hash.h>

#include "box2d/types.h"
#include "euler/physics/body.h"
#include "euler/physics/distance_joint.h"
#include "euler/physics/filter_joint.h"
#include "euler/physics/motor_joint.h"
#include "euler/physics/prismatic_joint.h"
#include "euler/physics/revolute_joint.h"
#include "euler/physics/util.h"
#include "euler/physics/weld_joint.h"
#include "euler/physics/wheel_joint.h"
#include "euler/physics/world.h"

using euler::physics::Joint;

static mrb_value
joint_type(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	switch (b2Joint_GetType(joint->id())) {
	case b2_distanceJoint: return EULER_SYM_VAL(distance);
	case b2_filterJoint: return EULER_SYM_VAL(filter);
	case b2_motorJoint: return EULER_SYM_VAL(motor);
	case b2_prismaticJoint: return EULER_SYM_VAL(prismatic);
	case b2_revoluteJoint: return EULER_SYM_VAL(revolute);
	case b2_weldJoint: return EULER_SYM_VAL(weld);
	case b2_wheelJoint: return EULER_SYM_VAL(wheel);
	default:
		state->rb().raise(state->rb().type_error(),
		    "Unknown joint type");
		std::unreachable();
	}
}

static mrb_value
joint_body_a(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	const b2BodyId body_id = b2Joint_GetBodyA(joint->id());
	// return box2d_body_wrap(mrb, body_id);
	// auto body =
	// euler::util::make_reference<euler::physics::Body>(body_id);
	auto body = euler::physics::Body::wrap(body_id);
	return state->wrap(body);
}

static mrb_value
joint_body_b(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	const b2BodyId body_id = b2Joint_GetBodyB(joint->id());
	auto body = euler::physics::Body::wrap(body_id);
	return state->wrap(body);
}

static mrb_value
joint_world(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	const b2WorldId world_id = b2Joint_GetWorld(joint->id());
	auto world = euler::physics::World::wrap(world_id);
	return state->wrap(world);
}

static mrb_value
joint_set_local_frame_a(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	mrb_value value;
	state->rb().get_args("H", &value);
	const b2Transform local_frame_a
	    = euler::physics::value_to_b2_transform(mrb, value);
	b2Joint_SetLocalFrameA(joint->id(), local_frame_a);
	return mrb_nil_value();
}

static mrb_value
joint_set_local_frame_b(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	mrb_value value;
	state->rb().get_args("H", &value);
	const b2Transform local_frame_b
	    = euler::physics::value_to_b2_transform(mrb, value);
	b2Joint_SetLocalFrameB(joint->id(), local_frame_b);
	return mrb_nil_value();
}

static mrb_value
joint_local_frame_a(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	const b2Transform local_frame_a = b2Joint_GetLocalFrameA(joint->id());
	return euler::physics::b2_transform_to_value(mrb, local_frame_a);
}

static mrb_value
joint_local_frame_b(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	const b2Transform local_frame_b = b2Joint_GetLocalFrameB(joint->id());
	return euler::physics::b2_transform_to_value(mrb, local_frame_b);
}

static mrb_value
joint_set_collide_connected(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	mrb_bool flag;
	state->rb().get_args("b", &flag);
	b2Joint_SetCollideConnected(joint->id(), flag);
	return mrb_nil_value();
}

static mrb_value
joint_collide_connected(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	const bool flag = b2Joint_GetCollideConnected(joint->id());
	return mrb_bool_value(flag);
}

static mrb_value
joint_wake_bodies(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	b2Joint_WakeBodies(joint->id());
	return mrb_nil_value();
}

static mrb_value
joint_constraint_force(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	const b2Vec2 force = b2Joint_GetConstraintForce(joint->id());
	return euler::physics::b2_vec_to_value(mrb, force);
}

static mrb_value
joint_linear_separation(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	const float separation = b2Joint_GetLinearSeparation(joint->id());
	return state->rb().float_value(separation);
}

static mrb_value
joint_angular_separation(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	const float separation = b2Joint_GetAngularSeparation(joint->id());
	return state->rb().float_value(separation);
}

static mrb_value
joint_set_constraint_tuning(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	/* should be a hash with keys `hertz` and `damping_ratio` */
	mrb_value tuning_value;
	state->rb().get_args("H", &tuning_value);
	const mrb_value hertz_value
	    = state->rb().hash_get(tuning_value, EULER_SYM_VAL(hertz));
	const mrb_value damping_ratio_value = state->rb().hash_get(
	    tuning_value, EULER_SYM_VAL(damping_ratio));
	const float hertz = (float)mrb_float(hertz_value);
	const float damping_ratio = (float)mrb_float(damping_ratio_value);
	b2Joint_SetConstraintTuning(joint->id(), hertz, damping_ratio);
	return mrb_nil_value();
}

static mrb_value
joint_constraint_tuning(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	float hertz = 0.0f;
	float damping_ratio = 0.0f;
	b2Joint_GetConstraintTuning(joint->id(), &hertz, &damping_ratio);
	const mrb_value out = state->rb().hash_new_capa(2);
	state->rb().hash_set(out, EULER_SYM_VAL(hertz),
	    state->rb().float_value(hertz));
	state->rb().hash_set(out, EULER_SYM_VAL(damping_ratio),
	    state->rb().float_value(damping_ratio));
	return out;
}

static mrb_value
joint_set_force_threshold(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	mrb_float value;
	state->rb().get_args("f", &value);
	b2Joint_SetForceThreshold(joint->id(), (float)value);
	return mrb_nil_value();
}

static mrb_value
joint_force_threshold(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	const float threshold = b2Joint_GetForceThreshold(joint->id());
	return state->rb().float_value(threshold);
}

static mrb_value
joint_set_torque_threshold(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	mrb_float value;
	state->rb().get_args("f", &value);
	b2Joint_SetTorqueThreshold(joint->id(), (float)value);
	return mrb_nil_value();
}

static mrb_value
joint_torque_threshold(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto joint = state->unwrap<Joint>(self);
	const float threshold = b2Joint_GetTorqueThreshold(joint->id());
	return state->rb().float_value(threshold);
}

RClass *
box2d_joint_init(mrb_state *mrb, RClass *mod)
{
	const auto state = euler::util::State::get(mrb);
	RClass *joint
	    = state->rb().define_class_under(mod, "Joint", mrb->object_class);
	MRB_SET_INSTANCE_TT(joint, MRB_TT_DATA);
	state->rb().define_method(joint, "type", joint_type, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "body_a", joint_body_a,
	    MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "body_b", joint_body_b,
	    MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "world", joint_world,
	    MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "local_frame_a=", joint_set_local_frame_a, MRB_ARGS_REQ(1));
	state->rb().define_method(joint,
	    "local_frame_b=", joint_set_local_frame_b, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "local_frame_a", joint_local_frame_a,
	    MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "local_frame_b", joint_local_frame_b,
	    MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "collide_connected=", joint_set_collide_connected, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "collide_connected",
	    joint_collide_connected, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "wake_bodies", joint_wake_bodies,
	    MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "constraint_force",
	    joint_constraint_force, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "linear_separation",
	    joint_linear_separation, MRB_ARGS_REQ(0));
	state->rb().define_method(joint, "angular_separation",
	    joint_angular_separation, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "constraint_tuning=", joint_set_constraint_tuning, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "constraint_tuning",
	    joint_constraint_tuning, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "force_threshold=", joint_set_force_threshold, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "force_threshold",
	    joint_force_threshold, MRB_ARGS_REQ(0));
	state->rb().define_method(joint,
	    "torque_threshold=", joint_set_torque_threshold, MRB_ARGS_REQ(1));
	state->rb().define_method(joint, "torque_threshold",
	    joint_torque_threshold, MRB_ARGS_REQ(0));
	return joint;
}

RClass *
Joint::init(const euler::util::Reference<euler::util::State> &state,
    RClass *mod, RClass *)
{
	return box2d_joint_init(state->rb().mrb(), mod);
}

mrb_value
Joint::Event::wrap(mrb_state *mrb) const
{
	auto state = euler::util::State::get(mrb);
	mrb_value out = state->rb().hash_new_capa(1);
	state->rb().hash_set(out, EULER_SYM_VAL(joint), joint->wrap(state));
	return out;
}

Joint::Event
Joint::Event::from_b2(const b2JointEvent &event)
{
	const auto state = fetch_state(event.jointId);
	const auto joint = Joint::wrap(event.jointId);
	return Event { joint };
}

Joint::Events
Joint::Events::from_b2(const b2JointEvents &events)
{
	std::vector<Event> joint_events;
	joint_events.reserve(events.count);
	for (int i = 0; i < events.count; ++i) {
		const b2JointEvent &b2_event = events.jointEvents[i];
		joint_events.push_back(Event::from_b2(b2_event));
	}
	return Events { joint_events };
}

mrb_value
Joint::Events::wrap(mrb_state *mrb) const
{
	const auto state = euler::util::State::get(mrb);
	const auto count = static_cast<mrb_int>(events.size());
	const auto ary = state->rb().ary_new_capa(count);
	for (const auto &event : events)
		state->rb().ary_push(ary, event.wrap(mrb));
	return ary;
}

euler::util::Reference<euler::util::State>
Joint::fetch_state(b2JointId id)
{
	const auto world_id = b2Joint_GetWorld(id);
	return World::fetch_state(world_id);
}

euler::util::Reference<euler::util::State>
Joint::state() const
{
	return fetch_state(_id);
}

euler::util::Reference<euler::physics::Body>
Joint::body_a() const
{
	const auto id = b2Joint_GetBodyA(_id);
	return Body::wrap(id);
}

euler::util::Reference<euler::physics::Body>
Joint::body_b() const
{
	const auto id = b2Joint_GetBodyB(_id);
	return Body::wrap(id);
}

euler::util::Reference<euler::physics::World>
Joint::world() const
{
	const auto id = b2Joint_GetWorld(_id);
	return World::wrap(id);
}

void
Joint::set_local_frame_a(b2Transform transform)
{
	b2Joint_SetLocalFrameA(_id, transform);
}

void
Joint::set_local_frame_b(b2Transform transform)
{
	b2Joint_SetLocalFrameB(_id, transform);
}

b2Transform
Joint::local_frame_a() const
{
	return b2Joint_GetLocalFrameA(_id);
}

b2Transform
Joint::local_frame_b() const
{
	return b2Joint_GetLocalFrameB(_id);
}

void
Joint::set_collide_connected(bool flag)
{
	b2Joint_SetCollideConnected(_id, flag);
}

bool
Joint::collide_connected() const
{
	return b2Joint_GetCollideConnected(_id);
}

void
Joint::wake_bodies() const
{
	b2Joint_WakeBodies(_id);
}

b2Vec2
Joint::constraint_force() const
{
	return b2Joint_GetConstraintForce(_id);
}

float
Joint::linear_separation() const
{
	return b2Joint_GetLinearSeparation(_id);
}

float
Joint::angular_separation() const
{
	return b2Joint_GetAngularSeparation(_id);
}

void
Joint::set_constraint_tuning(float hertz, float damping_ratio)
{
	b2Joint_SetConstraintTuning(_id, hertz, damping_ratio);
}

std::pair<float, float>
Joint::constraint_tuning() const
{
	float hertz = 0.0f;
	float damping_ratio = 0.0f;
	b2Joint_GetConstraintTuning(_id, &hertz, &damping_ratio);
	return { hertz, damping_ratio };
}

void
Joint::set_force_threshold(float threshold)
{
	b2Joint_SetForceThreshold(_id, threshold);
}

float
Joint::force_threshold() const
{
	return b2Joint_GetForceThreshold(_id);
}

void
Joint::set_torque_threshold(float threshold)
{
	b2Joint_SetTorqueThreshold(_id, threshold);
}

float
Joint::torque_threshold() const
{
	return b2Joint_GetTorqueThreshold(_id);
}

euler::util::Reference<Joint>
Joint::wrap(b2JointId id)
{
	const auto state = fetch_state(id);
	const auto data = b2Joint_GetUserData(id);
	if (data != nullptr) {
		auto joint = static_cast<Joint *>(data);
		return euler::util::Reference(joint);
	}
	switch (b2Joint_GetType(id)) {
	case b2_distanceJoint: {
		const auto joint = new DistanceJoint(id);
		return util::Reference(joint);
	}
	case b2_filterJoint: {
		const auto joint = new FilterJoint(id);
		return util::Reference(joint);
	}
	case b2_motorJoint: {
		const auto joint = new MotorJoint(id);
		return util::Reference(joint);
	}
	case b2_prismaticJoint: {
		const auto joint = new PrismaticJoint(id);
		return util::Reference(joint);
	}
	case b2_revoluteJoint: {
		const auto joint = new RevoluteJoint(id);
		return util::Reference(joint);
	}
	case b2_weldJoint: {
		const auto joint = new WeldJoint(id);
		return util::Reference(joint);
	}
	case b2_wheelJoint: {
		const auto joint = new WheelJoint(id);
		return util::Reference(joint);
	}
	default: std::unreachable();
	}
}
