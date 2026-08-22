/* SPDX-License-Identifier: ISC */

#include <utility>

#include "euler/physics/body.h"

#include "euler/physics/joint.h"
#include "euler/physics/shape.h"
#include "euler/physics/util.h"
#include "euler/physics/world.h"
#include "euler/util/ext.h"

using euler::physics::Body;

#define DEFINE_WAKE_KWARGS()                                                   \
	static const char *KW_NAMES[] = {                                      \
		"wake",                                                        \
	};                                                                     \
	enum {                                                                 \
		WAKE = 0,                                                      \
		KW_COUNT = sizeof(KW_NAMES) / sizeof(KW_NAMES[0]),             \
	};                                                                     \
	mrb_sym kw_syms[KW_COUNT];                                             \
	for (size_t i = 0; i < KW_COUNT; ++i) {                                \
		const size_t len = strlen(KW_NAMES[i]);                        \
		kw_syms[i] = state->rb().intern_static(KW_NAMES[i], len);    \
	}                                                                      \
	mrb_value kw_values[KW_COUNT];                                         \
	euler::physics::init_kw_values(kw_values);                             \
	const mrb_kwargs kwargs = {                                            \
		.num = KW_COUNT,                                               \
		.required = 0,                                                 \
		.table = kw_syms,                                              \
		.values = kw_values,                                           \
		.rest = NULL,                                                  \
	}

/**
 * @overload Euler::Physics::Body#angular_damping
 *   Get the current angular damping.
 *   @return [Numeric] The angular damping of the body.
 */
static mrb_value
body_angular_damping(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const float angular_damping = body->angular_damping();
	return state->rb().float_value(angular_damping);
}

/**
 * @overload Euler::Physics::Body#angular_damping=(value)
 *   Adjust the angular damping.
 *   @param value [Numeric] The new angular damping of the body.
 *   @return [void]
 */
static mrb_value
body_set_angular_damping(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_float value;
	state->rb().get_args("f", &value);
	body->set_angular_damping(static_cast<float>(value));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#angular_velocity
 *   Get the angular velocity of a body in radians per second.
 *   @return [Numeric] The angular velocity of the body.
 */
static mrb_value
body_angular_velocity(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const float angular_velocity = body->angular_velocity();
	return state->rb().float_value(angular_velocity);
}

/**
 * @overload Euler::Physics::Body#angular_velocity=(value)
 *   Set the angular velocity of a body in radians per second.
 *   @param value [Numeric] The new angular velocity of the body.
 *   @return [void]
 */
static mrb_value
body_set_angular_velocity(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_float value;
	state->rb().get_args("f", &value);
	body->set_angular_velocity(static_cast<float>(value));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#apply_angular_impulse(impulse, wake: false)
 *   Apply an angular impulse. The impulse is ignored if the body is not awake.
 *   This optionally wakes the body.
 *   @param impulse [Numeric] The angular impulse, usually in units of kg*m*m/s
 *   @param wake [Boolean] Also wake up the body (default: false)
 *   @return [void]
 */
static mrb_value
body_apply_angular_impulse(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	DEFINE_WAKE_KWARGS();
	const auto body = Body::unwrap(mrb, self);
	mrb_float impulse;
	mrb_bool wake = false;
	state->rb().get_args("f:", &impulse, &kwargs);
	if (!mrb_undef_p(kw_values[WAKE])) wake = mrb_bool(kw_values[WAKE]);
	body->apply_angular_impulse(static_cast<float>(impulse), wake);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#apply_force(force, point, wake: false)
 *   Apply a force at a world point. If the force is not applied at the center
 *   of mass, it will generate a torque and affect the angular velocity. This
 *   optionally wakes up the body. The force is ignored if the body is not
 *   awake.
 *   @param force [Array<Numeric>] The world force vector, usually in newtons
 *          (N)
 *   @param point [Array<Numeric>] The world position of the point of
 *          application
 *   @param wake [Boolean] Also wake up the body (default: false)
 *   @return [void]
 */
static mrb_value
body_apply_force(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	DEFINE_WAKE_KWARGS();
	const auto body = Body::unwrap(mrb, self);
	mrb_bool wake = false;
	mrb_value force_value;
	mrb_value point_value;
	state->rb().get_args("oo:", &force_value, &point_value, &kwargs);
	if (!mrb_undef_p(kw_values[WAKE])) wake = mrb_bool(kw_values[WAKE]);
	const b2Vec2 force = euler::physics::value_to_b2_vec(mrb, force_value);
	const b2Vec2 point = euler::physics::value_to_b2_vec(mrb, point_value);
	body->apply_force(force, point, wake);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#apply_force_to_center(force, wake: false)
 *   Apply a force to the center of mass. This optionally wakes up the body.
 *   @param force [Array<Numeric>] the world force vector, usually in newtons
 *          (N).
 *   @param wake [Boolean] Also wake up the body (default: false)
 *   @return [void]
 */
static mrb_value
body_apply_force_to_center(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	DEFINE_WAKE_KWARGS();
	const auto body = Body::unwrap(mrb, self);
	mrb_value force_value;
	mrb_bool wake = false;
	state->rb().get_args("o:", &force_value, &kwargs);
	if (!mrb_undef_p(kw_values[WAKE])) wake = mrb_bool(kw_values[WAKE]);
	const b2Vec2 force = euler::physics::value_to_b2_vec(mrb, force_value);
	body->apply_force_to_center(force, wake);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#apply_linear_impulse(impulse, point, wake:
 * false) Apply an impulse at a point. This immediately modifies the velocity.
 * It also modifies the angular velocity if the point of application is not at
 *   the center of mass. This optionally wakes the body. The impulse is ignored
 *   if the body is not awake.
 *   @param impulse [Array<Numeric>] The world impulse vector, usually in N*s or
 *          kg*m/s.
 *   @param point [Array<Numeric] The world position of the point of
 *          application.
 *   @param wake [Boolean] Also wake up the body (default: false)
 *   @return [void]
 */
static mrb_value
body_apply_linear_impulse(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	DEFINE_WAKE_KWARGS();
	const auto body = Body::unwrap(mrb, self);
	mrb_value impulse_value;
	mrb_value point_value;
	mrb_bool wake = false;
	state->rb().get_args("oo:", &impulse_value, &point_value, &kwargs);
	if (!mrb_undef_p(kw_values[WAKE])) wake = mrb_bool(kw_values[WAKE]);
	const b2Vec2 impulse
	    = euler::physics::value_to_b2_vec(mrb, impulse_value);
	const b2Vec2 point = euler::physics::value_to_b2_vec(mrb, point_value);
	body->apply_linear_impulse(impulse, point, wake);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#apply_linear_impulse_to_center(impulse, wake:
 * false) Apply an angular impulse. The impulse is ignored if the body is not
 * awake. This optionally wakes the body
 *   @param impulse [Array<Numeric>] The world impulse vector, usually in N*s or
 *          kg*m/s.
 *   @param wake [Boolean] Also wake up the body (default: false)
 *   @return [void]
 */
static mrb_value
body_apply_linear_impulse_to_center(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	DEFINE_WAKE_KWARGS();
	const auto body = Body::unwrap(mrb, self);
	mrb_value impulse_value;
	mrb_bool wake = false;
	state->rb().get_args("o:", &impulse_value, &kwargs);
	if (!mrb_undef_p(kw_values[WAKE])) wake = mrb_bool(kw_values[WAKE]);
	const b2Vec2 impulse
	    = euler::physics::value_to_b2_vec(mrb, impulse_value);
	body->apply_linear_impulse_to_center(impulse, wake);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#apply_mass_from_shapes
 *   This updates the mass properties to the sum of the mass properties of the
 *   shapes. This normally does not need to be called unless you called
 *   `#mass_data=` to override the mass and you later want to reset the mass.
 *   You may also use this when automatic mass computation has been disabled.
 *   You should call this regardless of body type.
 * @return [void]
 * @note Sensor shapes may have mass.
 */
static mrb_value
body_apply_mass_from_shapes(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	body->apply_mass_from_shapes();
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#apply_torque(torque, wake: false)
 *  Apply a torque. This affects the angular velocity without affecting the
 *   linear velocity. This optionally wakes the body. The torque is ignored if
 *   the body is not awake.
 *   @param torque [Numeric] Torque about the z-axis (out of the screen),
 *          usually in N*m.
 *   @param wake [Boolean] Also wake up the body (default: false)
 *   @return [void]
 */
static mrb_value
body_apply_torque(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	DEFINE_WAKE_KWARGS();
	const auto body = Body::unwrap(mrb, self);
	mrb_float torque;
	mrb_bool wake = false;
	state->rb().get_args("f|b", &torque, &kwargs);
	if (!mrb_undef_p(kw_values[WAKE])) wake = mrb_bool(kw_values[WAKE]);
	body->apply_torque(static_cast<float>(torque), wake);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#clear_forces
 *   Clear the force and torque on this body. Forces and torques are
 *   automatically cleared after each world step. So this only needs to be
 *   called if the application wants to remove the effect of previous calls to
 *   apply forces and torques before the world step is called.
 *   @return [void]
 */
static mrb_value
body_clear_forces(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	body->clear_forces();
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#compute_aabb
 *   Get the current world AABB that contains all the attached shapes. Note that
 *   this may not encompass the body origin. If there are no shapes attached
 *   then the returned AABB is empty and centered on the body origin.
 *   @return [Hash<Symbol, Array<Numeric>>] The AABB with :upper and :lower keys
 *           set to arrays representing the upper and lower bounds.
 */
static mrb_value
body_compute_aabb(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const b2AABB aabb = body->compute_aabb();
	const mrb_value upper
	    = euler::physics::b2_vec_to_value(mrb, aabb.upperBound);
	const mrb_value lower
	    = euler::physics::b2_vec_to_value(mrb, aabb.lowerBound);
	const mrb_value out = state->rb().hash_new_capa(2);
	state->rb().hash_set(out, EULER_SYM_VAL(upper), upper);
	state->rb().hash_set(out, EULER_SYM_VAL(lower), lower);
	return out;
}

/**
 * @overload Euler::Physics::Body#contact_data
 *   Get the touching contact data for a body.
 *   @return [Array<Hash>] An array of contact data hashes. Each hash has the
 *           following keys:
 *           - :contact [Euler::Physics::Contact] The contact.
 *           - :shapes [Array<Euler::Physics::Shape>] The two shapes involved in
 * the contact.
 *           - :manifold [Hash] The contact manifold data. This contains the
 *             following keys:
 *             - :normal [Array<Numeric>] The unit normal vector in world
 *               space, points from shape A to body B.
 *             - :rolling_impulse [Numeric] Angular impulse applied for
 *               rolling resistance. N*m*s = kg*m^2/s
 *             - :points [Array<Hash>] An array of 0-2 contact points. Each
 *               point hash has the following keys:
 *               - :point [Array<Numeric>] Location of the contact point in
 *                 world space. Subject to precision loss at large
 *                 coordinates.
 *               - :anchors [Array<Array<Numeric>>] An array of two anchor
 *                 points, containing the contact point relative to each
 *                 shape's origin in world space.
 *               - :separation [Numeric] The separation of the contact point,
 *                 negative if penetrating.
 *               - :normal_impulse [Numeric] The impulse along the manifold
 *                 normal vector.
 *               - :tangent_impulse [Numeric] The friction impulse.
 *               - :total_normal_impulse [Numeric] The total normal impulse
 *                  applied across sub-stepping and restitution. This is
 *                  important to identify speculative contact points that had
 *                  an interaction in the time step.
 *               - :normal_velocity [Numeric] Relative normal velocity
 *                 pre-solve. Used for hit events. If the normal impulse is
 *                 zero then there was no hit. Negative means shapes are
 *                 approaching.
 *               - :id [Integer] Uniquely identifies a contact point between
 *                 two shapes.
 *               - :persisted [Boolean] Did this contact point exist the
 *                 previous step?
 */
static mrb_value
body_contact_data(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const auto data = body->contact_data();
	const mrb_value out = state->rb().ary_new_capa(data.size());
	for (const auto &entry : data) {
		auto cd = euler::physics::Contact::Data::from_b2(entry);
		state->rb().ary_push(out, cd.wrap(mrb));
	}
	return out;
}

/**
 * @overload Euler::Physics::Body#disable
 *   Disable a body by removing it completely from the simulation. This is
 *   expensive.
 *   @return [void]
 */
static mrb_value
body_disable(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	body->disable();
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#enable
 *   Enable a body by adding it to the simulation. This is expensive.
 *   @return [void]
 */
static mrb_value
body_enable(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	body->enable();
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#contact_events_enabled=(enable)
 *   Enable contact events for this shape. Only applies to kinematic and dynamic
 *   bodies. Ignored for sensors.
 *   @param enable [Boolean] true to enable contact events.
 *   @return [void]
 */
static mrb_value
body_enable_contact_events(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_bool enable;
	state->rb().get_args("b", &enable);
	body->enable_contact_events(enable);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#hit_events_enabled=(enable)
 *   Enable contact hit events for this shape. Ignored for sensors.
 *   @param enable [Boolean] true to enable hit events.
 *   @return [void]
 */
static mrb_value
body_enable_hit_events(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_bool enable;
	state->rb().get_args("b", &enable);
	body->enable_hit_events(enable);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#sleep_enabled?
 *   @see Euler::Physics::Body#sleep_enabled
 * @overload Euler::Physics::Body#sleep_enabled
 *   Check if sleeping is enabled for this body.
 *   @return [Boolean] true if sleeping is enabled.
 */
static mrb_value
body_set_sleep_enabled(mrb_state *mrb, mrb_value self)
{
	// const auto state = euler::util::State::get(mrb);
	// const auto body = Body::unwrap(mrb, self);
	// const bool enabled = body->is_sleep_enabled();
	// return mrb_bool_value(enabled);
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_bool enable;
	state->rb().get_args("b", &enable);
	body->set_sleep_enabled(enable);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#gravity_scale=(value)
 *   Adjust the gravity scale.
 *   @param value [Numeric] The new gravity scale of the body.
 *   @return [void]
 */
static mrb_value
body_set_gravity_scale(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_float value;
	state->rb().get_args("f", &value);
	body->set_gravity_scale(static_cast<float>(value));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#gravity_scale
 *   Get the current gravity scale.
 */
static mrb_value
body_gravity_scale(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const float gravity_scale = body->gravity_scale();
	return state->rb().float_value(gravity_scale);
}

/**
 * @overload Euler::Physics::Body#awake?
 *   @see Euler::Physics::Body#awake
 * @overload Euler::Physics::Body#awake
 *   Check if the body is awake.
 *   @return [Boolean] true if the body is awake.
 */
static mrb_value
body_awake(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const bool awake = body->is_awake();
	return mrb_bool_value(awake);
}

/**
 * @overload Euler::Physics::Body#awake=(awake)
 *   Wake a body from sleep. This wakes the entire island the body is touching.
 *   @return [void]
 */
static mrb_value
body_set_awake(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_bool awake;
	state->rb().get_args("b", &awake);
	body->set_awake(awake);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#bullet?
 *   @see Euler::Physics::Body#bullet
 * @overload Euler::Physics::Body#bullet
 *   Is this body a bullet?
 *   @return [Boolean] true if the body is a bullet.
 */
static mrb_value
body_is_bullet(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const bool is_bullet = body->is_bullet();
	return mrb_bool_value(is_bullet);
}

/**
 * @overload Euler::Physics::Body#bullet=(flag)
 *   Set this body to be a bullet. A bullet does continuous collision detection
 *   against dynamic bodies (but not other bullets).
 *   @param flag [Boolean] true to set the body as a bullet.
 *   @return [void]
 */
static mrb_value
body_set_bullet(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_bool flag;
	state->rb().get_args("b", &flag);
	body->set_bullet(flag);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#enabled?
 *   @see Euler::Physics::Body#enabled
 * @overload Euler::Physics::Body#enabled
 *   Check if the body is enabled.
 *   @return [Boolean] true if the body is enabled.
 */
static mrb_value
body_is_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const bool enabled = body->is_enabled();
	return mrb_bool_value(enabled);
}

static mrb_value
body_set_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_bool enable;
	state->rb().get_args("b", &enable);
	if (enable) body->enable();
	else body->disable();
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#sleep_enabled?
 *   @see Euler::Physics::Body#sleep_enabled
 * @overload Euler::Physics::Body#sleep_enabled
 *   Check if sleeping is enabled for this body.
 *   @return [Boolean] true if sleeping is enabled.
 */
static mrb_value
body_sleep_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const bool enabled = body->is_sleep_enabled();
	return mrb_bool_value(enabled);
}

/**
 * @overload Euler::Physics::Body#valid?
 *   Check if the body is valid.
 *   @return [Boolean] true if the body is valid.
 */
static mrb_value
body_is_valid(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const bool valid = body->is_valid();
	return mrb_bool_value(valid);
}

/**
 * @overload Euler::Physics::Body#joints
 *   Get all joints on this body.
 *   @return [Array<Euler::Physics::Joint>] An array of joints attached to this
 * body.
 */
static mrb_value
body_joints(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	auto joints = body->joints();
	const mrb_value out = state->rb().ary_new_capa(joints.size());
	for (const auto &joint : joints)
		state->rb().ary_push(out, joint->wrap(state));
	return out;
}

/**
 * @overload Euler::Physics::Body#linear_damping
 *   Get the current linear damping.
 *   @return [Numeric] The linear damping of the body.
 */
static mrb_value
body_linear_damping(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const float linear_damping = body->linear_damping();
	return state->rb().float_value(linear_damping);
}

/**
 * @overload Euler::Physics::Body#linear_damping=(value)
 *   Adjust the linear damping.
 *   @param value [Numeric] The new linear damping of the body.
 *   @return [void]
 */
static mrb_value
body_set_linear_damping(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_float value;
	state->rb().get_args("f", &value);
	body->set_linear_damping(static_cast<float>(value));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#linear_velocity
 *   Get the linear velocity of a body.
 *   @return [Array<Numeric>] The linear velocity vector of the body.
 */
static mrb_value
body_linear_velocity(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const b2Vec2 linear_velocity = body->linear_velocity();
	return euler::physics::b2_vec_to_value(mrb, linear_velocity);
}

/**
 * @overload Euler::Physics::Body#linear_velocity=(value)
 *   Set the linear velocity of a body.
 *   @param value [Array<Numeric>] The new linear velocity vector of the body.
 *   @return [void]
 */
static mrb_value
body_set_linear_velocity(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_value value;
	state->rb().get_args("o", &value);
	const b2Vec2 linear_velocity
	    = euler::physics::value_to_b2_vec(mrb, value);
	body->set_linear_velocity(linear_velocity);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#local_center_of_mass
 *   Get the local position of the center of mass.
 *   @return [Array<Numeric>] The local position of the center of mass.
 */
static mrb_value
body_local_center_of_mass(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const b2Vec2 local_com = body->local_center_of_mass();
	return euler::physics::b2_vec_to_value(mrb, local_com);
}

/**
 * @overload Euler::Physics::Body#local_point(point)
 *   Get a local point given a world point.
 *   @param point [Array<Numeric>] The world point.
 *   @return [Array<Numeric>] The corresponding local point.
 */
static mrb_value
body_local_point(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_value point_value;
	state->rb().get_args("o", &point_value);
	const b2Vec2 point = euler::physics::value_to_b2_vec(mrb, point_value);
	const b2Vec2 local_point = body->local_point(point);
	return euler::physics::b2_vec_to_value(mrb, local_point);
}

/**
 * @overload Euler::Physics::Body#local_point_velocity(point)
 *   Get the velocity of a local point.
 *   @param point [Array<Numeric>] The local point.
 */
static mrb_value
body_local_point_velocity(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_value point_value;
	state->rb().get_args("o", &point_value);
	const b2Vec2 point = euler::physics::value_to_b2_vec(mrb, point_value);
	const b2Vec2 local_point_velocity = body->local_point_velocity(point);
	return euler::physics::b2_vec_to_value(mrb, local_point_velocity);
}

/**
 * @overload Euler::Physics::Body#local_vector(vector)
 *   Get a local vector given a world vector.
 *   @param vector [Array<Numeric>] The world vector.
 *   @return [Array<Numeric>] The corresponding local vector.
 */
static mrb_value
body_local_vector(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_value vector_value;
	state->rb().get_args("o", &vector_value);
	const b2Vec2 vector
	    = euler::physics::value_to_b2_vec(mrb, vector_value);
	const b2Vec2 local_vector = body->local_vector(vector);
	return euler::physics::b2_vec_to_value(mrb, local_vector);
}

/**
 * @overload Euler::Physics::Body#mass
 *   Get the mass of the body.
 *   @return [Numeric] The mass of the body.
 */
static mrb_value
body_mass(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const float mass = body->mass();
	return state->rb().float_value(mass);
}

/**
 * @overload Euler::Physics::Body#mass_data
 *   Get the mass data of the body.
 *   @return [Hash<Symbol, Object>] The mass data hash with the following keys:
 *           - :center [Array<Numeric>] The center of mass in local
 *             coordinates.
 *           - :mass [Numeric] The mass of the body.
 *           - :rotational_inertia [Numeric] The rotational inertia of the
 *             body.
 */
static mrb_value
body_mass_data(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const b2MassData md = body->mass_data();
	const mrb_value center
	    = euler::physics::b2_vec_to_value(mrb, md.center);
	const mrb_value out = state->rb().hash_new_capa(3);
	const mrb_value mass = state->rb().float_value(md.mass);
	const mrb_value inertia
	    = state->rb().float_value(md.rotationalInertia);
	state->rb().hash_set(out, EULER_SYM_VAL(center), center);
	state->rb().hash_set(out, EULER_SYM_VAL(mass), mass);
	state->rb().hash_set(out, EULER_SYM_VAL(rotational_inertia), inertia);
	return out;
}

/**
 * @overload Euler::Physics::Body#mass_data=(mass_data)
 *   Override the body's mass properties. Normally this is computed
 *   automatically using the shape geometry and density. This information is
 *   lost if a shape is added or removed or if the body type changes.
 *   @param mass_data [Hash<Symbol, Object>] The mass data hash with the
 *          following keys:
 *           - :center [Array<Numeric>] The center of mass in local
 *             coordinates.
 *           - :mass [Numeric] The mass of the body.
 *           - :rotational_inertia [Numeric] The rotational inertia of the
 *             body.
 *   @return [void]
 */
static mrb_value
body_set_mass_data(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_value mdh;
	state->rb().get_args("o", &mdh);
	b2MassData md;
	const mrb_value center_value
	    = state->rb().hash_get(mdh, EULER_SYM_VAL(center));
	md.center = euler::physics::value_to_b2_vec(mrb, center_value);
	const mrb_value mass_value
	    = state->rb().hash_get(mdh, EULER_SYM_VAL(mass));
	md.mass = static_cast<float>(mrb_float(mass_value));
	const mrb_value inertia_value
	    = state->rb().hash_get(mdh, EULER_SYM_VAL(rotational_inertia));
	md.rotationalInertia = static_cast<float>(mrb_float(inertia_value));
	body->set_mass_data(md);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#motion_locks
 *   Get the motion locks for this body.
 *   @return [Hash<Symbol, Boolean>] The motion locks hash with the following
 *           keys:
 *           - :linear_x [Boolean] true to prevent translation along the x-axis.
 *           - :linear_y [Boolean] true to prevent translation along the y-axis.
 *           - :angular_z [Boolean] true to prevent rotation about the z-axis.
 */
static mrb_value
body_motion_locks(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const b2MotionLocks locks = body->motion_locks();
	const mrb_value out = state->rb().hash_new_capa(3);
	state->rb().hash_set(out, EULER_SYM_VAL(linear_x),
	    mrb_bool_value(locks.linearX));
	state->rb().hash_set(out, EULER_SYM_VAL(linear_y),
	    mrb_bool_value(locks.linearY));
	state->rb().hash_set(out, EULER_SYM_VAL(angular_z),
	    mrb_bool_value(locks.angularZ));
	return out;
}

/**
 * @overload Euler::Physics::Body#motion_locks=(locks)
 *   Set the motion locks for this body.
 *   @param locks [Hash<Symbol, Boolean>] The motion locks hash with the
 *          following keys:
 *           - :linear_x [Boolean] true to prevent translation along the x-axis.
 *           - :linear_y [Boolean] true to prevent translation along the y-axis.
 *           - :angular_z [Boolean] true to prevent rotation about the z-axis.
 *   @return [void]
 */
static mrb_value
body_set_motion_locks(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_value locks_value;
	state->rb().get_args("H", &locks_value);
	b2MotionLocks locks = {};
	const mrb_value linear_x_value
	    = state->rb().hash_get(locks_value, EULER_SYM_VAL(linear_x));
	locks.linearX = mrb_bool(linear_x_value);
	const mrb_value linear_y_value
	    = state->rb().hash_get(locks_value, EULER_SYM_VAL(linear_y));
	locks.linearY = mrb_bool(linear_y_value);
	const mrb_value angular_z_value
	    = state->rb().hash_get(locks_value, EULER_SYM_VAL(angular_z));
	locks.angularZ = mrb_bool(angular_z_value);
	body->set_motion_locks(locks);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#name
 *   Get the name of the body.
 *   @return [String] The name of the body.
 */
static mrb_value
body_name(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const char *name = body->name();
	return state->rb().str_new_cstr(name);
}

/**
 * @overload Euler::Physics::Body#name=(name)
 *   Set the name of the body.
 *   @param name [String] The name of the body.
 *   @return [void]
 */
static mrb_value
body_set_name(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const char *name;
	state->rb().get_args("z", &name);
	body->set_name(name);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#position
 * Get the position of the body.
 * @return [Array<Numeric>] The position vector of the body.
 */
static mrb_value
body_position(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const b2Vec2 position = body->position();
	return euler::physics::b2_vec_to_value(mrb, position);
}

/**
 * @overload Euler::Physics::Body#rotation
 *   Get the rotation of the body.
 *   @return [Numeric] The rotation of the body in radians.
 */
static mrb_value
body_rotation(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const b2Rot rotation = body->rotation();
	return euler::physics::b2_rot_to_value(mrb, rotation);
}

/**
 * @overload Euler::Physics::Body#rotational_inertia
 *   Get the rotational inertia of the body, usually in kg*m^2.
 *   @return [Numeric] The rotational inertia of the body.
 */
static mrb_value
body_rotational_inertia(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const float inertia = body->rotational_inertia();
	return state->rb().float_value(inertia);
}

/**
 * @overload Euler::Physics::Body#target_transform(transform, step: 1.0)
 *   Set the velocity to reach the given transform after a given time step. The
 *   result will be close but maybe not exact. This is meant for kinematic
 *   bodies. The target is not applied if the velocity would be below the sleep
 *   threshold. This will automatically wake the body if asleep.
 *   @param transform [Hash] The target transform hash with the following keys:
 *           - :position [Array<Numeric>] The position vector.
 *           - :rotation [Numeric] The rotation in radians.
 *   @param step [Numeric] The time step over which to achieve the target
 *          transform (default: 1.0).
 *   @return [void]
 */
static mrb_value
body_target_transform(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	DEFINE_WAKE_KWARGS();
	const auto body = Body::unwrap(mrb, self);
	mrb_value tform_value;
	mrb_float step = 1.0;
	state->rb().get_args("o|f:", &tform_value, &step, &kwargs);
	bool wake = mrb_undef_p(kw_values[0]) ? false : mrb_bool(kw_values[0]);
	const b2Transform tform
	    = euler::physics::value_to_b2_transform(mrb, tform_value);
	body->target_transform(tform, static_cast<float>(step), wake);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#shapes
 *   Get all shapes attached to this body.
 *   @return [Array<Euler::Physics::Shape>] An array of shapes attached to this
 * body.
 */
static mrb_value
body_shapes(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const auto shapes = body->shapes();
	const mrb_value out = state->rb().ary_new_capa(shapes.size());
	for (auto shape : shapes)
		state->rb().ary_push(out, state->wrap(shape));
	return out;
}

/**
 * @overload Euler::Physics::Body#sleep_threshold
 *   Get the sleep threshold for this body, usually in meters per second.
 *   @return [Numeric] The sleep threshold.
 */
static mrb_value
body_sleep_threshold(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const float sleep_threshold = body->sleep_threshold();
	return state->rb().float_value(sleep_threshold);
}

/**
 * @overload Euler::Physics::Body#sleep_threshold=(value)
 *   Set the sleep threshold for this body, usually in meters per second.
 *   @param value [Numeric] The new sleep threshold.
 *   @return [void]
 */
static mrb_value
body_set_sleep_threshold(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_float value;
	state->rb().get_args("f", &value);
	body->set_sleep_threshold(static_cast<float>(value));
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#transform
 *   Get the current transform of the body.
 *   @return [Hash] The transform hash with the following keys:
 *           - :position [Array<Numeric>] The position vector.
 *           - :rotation [Numeric] The rotation in radians.
 */
static mrb_value
body_transform(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const b2Transform transform = body->transform();
	return euler::physics::b2_transform_to_value(mrb, transform);
}

/**
 * @overload Euler::Physics::Body#transform=(transform)
 *   Set the world transform of the body. This acts as a teleport and is
 *   fairly expensive.
 *   @note Generally you should create a body with the intended transform.
 *   @param transform [Hash] The transform hash with the following keys:
 *           - :position [Array<Numeric>] The position vector.
 *           - :rotation [Numeric] The rotation in radians.
 *   @return [void]
 */
static mrb_value
body_set_transform(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_value tform_value;
	state->rb().get_args("o", &tform_value);
	const b2Transform tform
	    = euler::physics::value_to_b2_transform(mrb, tform_value);
	body->set_transform(tform.p, tform.q);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#type
 *   Get the body type.
 *   @return [Symbol] The body type, one of :static, :kinematic, or :dynamic.
 */
static mrb_value
body_type(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	switch (body->type()) {
	case b2_staticBody: return EULER_SYM_VAL(static);
	case b2_kinematicBody: return EULER_SYM_VAL(kinematic);
	case b2_dynamicBody: return EULER_SYM_VAL(dynamic);
	default:
		state->rb().raise(state->rb().type_error(),
		    "unknown body type");
		std::unreachable();
	}
}

static b2BodyType
sym_to_body_type(mrb_state *mrb, const mrb_sym type_sym)
{
	const auto state = euler::util::State::get(mrb);
	const mrb_value type = mrb_symbol_value(type_sym);
	if (state->rb().equal(type, EULER_SYM_VAL(static)))
		return b2_staticBody;
	if (state->rb().equal(type, EULER_SYM_VAL(kinematic)))
		return b2_kinematicBody;
	if (state->rb().equal(type, EULER_SYM_VAL(dynamic)))
		return b2_dynamicBody;
	state->rb().raise(state->rb().argument_error(),
	    "invalid body type");
	std::unreachable();
}

/**
 * @overload Euler::Physics::Body#type=(type)
 *   Set the body type.
 *   @param type [Symbol] The body type, one of :static, :kinematic, or
 *          :dynamic.
 *   @return [void]
 */
static mrb_value
body_set_type(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_sym type_sym;
	state->rb().get_args("n", &type_sym);
	const b2BodyType type = sym_to_body_type(mrb, type_sym);
	body->set_type(type);
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#wake_touching
 *   Wake up all bodies touching this body.
 *   @return [void]
 */
static mrb_value
body_wake_touching(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	body->wake_touching();
	return mrb_nil_value();
}

/**
 * @overload Euler::Physics::Body#world
 *   Get the world this body belongs to.
 *   @return [Euler::Physics::World] The world this body belongs to.
 */
static mrb_value
body_world(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	return body->world()->wrap(state);
}

/**
 * @overload Euler::Physics::Body#world_center_of_mass
 *   Get the world position of the center of mass.
 */
static mrb_value
body_world_center_of_mass(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	const b2Vec2 world_com = body->world_center_of_mass();
	return euler::physics::b2_vec_to_value(mrb, world_com);
}

/**
 * @overload Euler::Physics::Body#world_point(point)
 *   Get a world point given a local point.
 *   @param point [Array<Numeric>] The local point.
 *   @return [Array<Numeric>] The corresponding world point.
 */
static mrb_value
body_world_point(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_value point_value;
	state->rb().get_args("o", &point_value);
	const b2Vec2 point = euler::physics::value_to_b2_vec(mrb, point_value);
	const b2Vec2 world_point = body->world_point(point);
	return euler::physics::b2_vec_to_value(mrb, world_point);
}

/**
 * @overload Euler::Physics::Body#world_point_velocity(point)
 *   Get the velocity of a world point attached to this body. Usually in meters
 *   per second.
 *   @param point [Array<Numeric>] The world point
 *   @return [Array<Numeric>] The velocity vector of the world point.
 */
static mrb_value
body_world_point_velocity(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_value point_value;
	state->rb().get_args("o", &point_value);
	const b2Vec2 point = euler::physics::value_to_b2_vec(mrb, point_value);
	const b2Vec2 world_point_velocity = body->world_point_velocity(point);
	return euler::physics::b2_vec_to_value(mrb, world_point_velocity);
}

/**
 * @overload Euler::Physics::Body#world_vector(vector)
 *   Get a world vector given a local vector.
 *   @param vector [Array<Numeric>] The local vector.
 *   @return [Array<Numeric>] The corresponding world vector.
 */
static mrb_value
body_world_vector(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto body = Body::unwrap(mrb, self);
	mrb_value vector_value;
	state->rb().get_args("o", &vector_value);
	const b2Vec2 vector
	    = euler::physics::value_to_b2_vec(mrb, vector_value);
	const b2Vec2 world_vector = body->world_vector(vector);
	return euler::physics::b2_vec_to_value(mrb, world_vector);
}

RClass *
Body::init(const util::Reference<util::State> &state, RClass *mod, RClass *)
{
	auto mrb = state->mrb();
	auto cls = mrb->define_class_under(mod, "Body", state->object_class());
	mrb->define_method(cls, "angular_damping", body_angular_damping,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "angular_damping=", body_set_angular_damping,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "angular_velocity", body_angular_velocity,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "angular_velocity=", body_set_angular_velocity,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "apply_angular_impulse",
	    body_apply_angular_impulse, MRB_ARGS_REQ(1) | MRB_ARGS_KEY(1, 0));
	mrb->define_method(cls, "apply_force", body_apply_force,
	    MRB_ARGS_REQ(2) | MRB_ARGS_KEY(1, 0));
	mrb->define_method(cls, "apply_force_to_center",
	    body_apply_force_to_center, MRB_ARGS_REQ(1) | MRB_ARGS_KEY(1, 0));
	mrb->define_method(cls, "apply_linear_impulse",
	    body_apply_linear_impulse, MRB_ARGS_REQ(2) | MRB_ARGS_KEY(1, 0));
	mrb->define_method(cls, "apply_linear_impulse_to_center",
	    body_apply_linear_impulse_to_center,
	    MRB_ARGS_REQ(1) | MRB_ARGS_KEY(1, 0));
	mrb->define_method(cls, "apply_mass_from_shapes",
	    body_apply_mass_from_shapes, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "apply_torque", body_apply_torque,
	    MRB_ARGS_REQ(1) | MRB_ARGS_KEY(1, 0));
	mrb->define_method(cls, "clear_forces", body_clear_forces,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "compute_aabb", body_compute_aabb,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "contact_data", body_contact_data,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "disable", body_disable, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "enable", body_enable, MRB_ARGS_REQ(0));
	mrb->define_method(cls,
	    "contact_events_enabled=", body_enable_contact_events,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "hit_events_enabled=", body_enable_hit_events,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "sleep_enabled", body_sleep_enabled,
	    MRB_ARGS_REQ(0));
	mrb->alias_method(cls, mrb->intern_cstr("sleep_enabled?"),
	    mrb->intern_cstr("sleep_enabled"));
	mrb->define_method(cls, "sleep_enabled=", body_set_sleep_enabled,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "gravity_scale", body_gravity_scale,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "gravity_scale=", body_set_gravity_scale,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "awake", body_awake, MRB_ARGS_REQ(0));
	mrb->alias_method(cls, mrb->intern_cstr("awake?"),
	    mrb->intern_cstr("awake"));
	mrb->define_method(cls, "awake=", body_set_awake, MRB_ARGS_REQ(1));
	mrb->define_method(cls, "bullet", body_is_bullet, MRB_ARGS_REQ(0));
	mrb->alias_method(cls, mrb->intern_cstr("bullet?"),
	    mrb->intern_cstr("bullet"));
	mrb->define_method(cls, "bullet=", body_set_bullet, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "enabled", body_is_enabled, MRB_ARGS_REQ(0));
	mrb->alias_method(cls, mrb->intern_cstr("enabled?"),
	    mrb->intern_cstr("enabled"));
	mrb->define_method(cls, "enabled=", body_set_enabled, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "valid?", body_is_valid, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "joints", body_joints, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "linear_damping", body_linear_damping,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "linear_damping=", body_set_linear_damping,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "linear_velocity", body_linear_velocity,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "linear_velocity=", body_set_linear_velocity,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "local_center_of_mass",
	    body_local_center_of_mass, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "local_point", body_local_point,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "local_point_velocity",
	    body_local_point_velocity, MRB_ARGS_REQ(1));
	mrb->define_method(cls, "local_vector", body_local_vector,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "mass", body_mass, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "mass_data", body_mass_data, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "mass_data=", body_set_mass_data,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "motion_locks", body_motion_locks,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "motion_locks=", body_set_motion_locks,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "name", body_name, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "name=", body_set_name, MRB_ARGS_REQ(1));
	mrb->define_method(cls, "position", body_position, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "rotation", body_rotation, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "rotational_inertia", body_rotational_inertia,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "target_transform", body_target_transform,
	    MRB_ARGS_REQ(1) | MRB_ARGS_KEY(1, 0));
	mrb->define_method(cls, "shapes", body_shapes, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "sleep_threshold", body_sleep_threshold,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "sleep_threshold=", body_set_sleep_threshold,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "transform", body_transform, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "transform=", body_set_transform,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "type", body_type, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "type=", body_set_type, MRB_ARGS_REQ(1));
	mrb->define_method(cls, "wake_touching", body_wake_touching,
	    MRB_ARGS_REQ(0));
	mrb->define_method(cls, "world", body_world, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "world_center_of_mass",
	    body_world_center_of_mass, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "world_point", body_world_point,
	    MRB_ARGS_REQ(1));
	mrb->define_method(cls, "world_point_velocity",
	    body_world_point_velocity, MRB_ARGS_REQ(1));
	mrb->define_method(cls, "world_vector", body_world_vector,
	    MRB_ARGS_REQ(1));
	return cls;
}

euler::util::Reference<Body>
Body::wrap(b2BodyId id)
{
	const auto user_data = b2Body_GetUserData(id);
	if (user_data != nullptr) {
		auto body = static_cast<Body *>(user_data);
		return euler::util::Reference<Body>(body);
	}
	auto body = new Body(id);
	b2Body_SetUserData(id, body);
	return euler::util::Reference<Body>(body);
}

b2BodyType
Body::parse_type(mrb_state *mrb, mrb_value value)
{
	auto state = euler::util::State::get(mrb);
	if (state->rb().equal(value, EULER_SYM_VAL(static))) {
		return b2_staticBody;
	}
	if (state->rb().equal(value, EULER_SYM_VAL(kinematic))) {
		return b2_kinematicBody;
	}
	if (state->rb().equal(value, EULER_SYM_VAL(dynamic))) {
		return b2_dynamicBody;
	}
	state->rb().raise(state->rb().argument_error(),
	    "invalid body type");
	std::unreachable();
}

mrb_value
Body::MoveEvent::wrap(mrb_state *mrb)
{
	const auto state = euler::util::State::get(mrb);
	const auto hash = state->rb().hash_new_capa(3);
	const auto tform = b2_transform_to_value(mrb, transform);
	const auto asleep = mrb_bool_value(fell_asleep);
	state->rb().hash_set(hash, EULER_SYM_VAL(transform), tform);
	state->rb().hash_set(hash, EULER_SYM_VAL(body), state->wrap(body));
	state->rb().hash_set(hash, EULER_SYM_VAL(fell_asleep), asleep);
	return hash;
}

Body::MoveEvent
Body::MoveEvent::from_b2(const b2BodyMoveEvent &event)
{
	const auto world_id = b2Body_GetWorld(event.bodyId);
	const auto state = World::fetch_state(world_id);
	MoveEvent move_event;
	move_event.body = Body::wrap(event.bodyId);
	move_event.transform = event.transform;
	move_event.fell_asleep = event.fellAsleep;
	return move_event;
}

float
Body::angular_damping()
{
	return b2Body_GetAngularDamping(_id);
}

float
Body::angular_velocity()
{
	return b2Body_GetAngularVelocity(_id);
}

void
Body::apply_angular_impulse(float value, bool wake)
{
	b2Body_ApplyAngularImpulse(_id, value, wake);
}

void
Body::apply_force(b2Vec2 force, b2Vec2 point, bool wake)
{
	b2Body_ApplyForce(_id, force, point, wake);
}

void
Body::apply_force_to_center(b2Vec2 force, bool wake)
{
	b2Body_ApplyForceToCenter(_id, force, wake);
}

void
Body::apply_linear_impulse(b2Vec2 impulse, b2Vec2 point, bool wake)
{
	b2Body_ApplyLinearImpulse(_id, impulse, point, wake);
}

void
Body::apply_linear_impulse_to_center(b2Vec2 impulse, bool wake)
{
	b2Body_ApplyLinearImpulseToCenter(_id, impulse, wake);
}

void
Body::apply_mass_from_shapes()
{
	b2Body_ApplyMassFromShapes(_id);
}

void
Body::apply_torque(float torque, bool wake)
{
	b2Body_ApplyTorque(_id, torque, wake);
}

bool
Body::is_awake()
{
	return b2Body_IsAwake(_id);
}

void
Body::clear_forces()
{
	b2Body_ClearForces(_id);
}

b2AABB
Body::compute_aabb()
{
	return b2Body_ComputeAABB(_id);
}

std::vector<b2ContactData>
Body::contact_data()
{
	std::vector<b2ContactData> contacts;
	const int32_t count = b2Body_GetContactCapacity(_id);
	contacts.reserve(static_cast<size_t>(count));
	b2Body_GetContactData(_id, contacts.data(), count);
	return contacts;
}

void
Body::disable()
{
	b2Body_Disable(_id);
}

void
Body::enable()
{
	b2Body_Enable(_id);
}

void
Body::enable_contact_events(bool enable)
{
	b2Body_EnableContactEvents(_id, enable);
}

void
Body::enable_hit_events(bool enable)
{
	b2Body_EnableHitEvents(_id, enable);
}

float
Body::gravity_scale()
{
	return b2Body_GetGravityScale(_id);
}

bool
Body::is_bullet()
{
	return b2Body_IsBullet(_id);
}

bool
Body::is_enabled()
{
	return b2Body_IsEnabled(_id);
}

bool
Body::is_sleep_enabled()
{
	return b2Body_IsSleepEnabled(_id);
}

bool
Body::is_valid()
{
	return b2Body_IsValid(_id);
}

std::vector<euler::util::Reference<euler::physics::Joint>>
Body::joints()
{
	std::vector<b2JointId> b2_joints;
	const auto count = b2Body_GetJointCount(_id);
	b2_joints.resize(static_cast<size_t>(count));
	b2Body_GetJoints(_id, b2_joints.data(), count);
	std::vector<util::Reference<Joint>> joints;
	joints.reserve(static_cast<size_t>(count));
	for (const auto &b2_joint : b2_joints) {
		auto joint = Joint::wrap(b2_joint);
		joints.emplace_back(joint);
	}
	return joints;
}

float
Body::linear_damping()
{
	return b2Body_GetLinearDamping(_id);
}

b2Vec2
Body::linear_velocity()
{
	return b2Body_GetLinearVelocity(_id);
}

b2Vec2
Body::local_center_of_mass()
{
	return b2Body_GetLocalCenterOfMass(_id);
}

b2Vec2
Body::local_point(b2Vec2 point)
{
	return b2Body_GetLocalPoint(_id, point);
}

b2Vec2
Body::local_point_velocity(b2Vec2 point)
{
	return b2Body_GetLocalPointVelocity(_id, point);
}

b2Vec2
Body::local_vector(b2Vec2 vector)
{
	return b2Body_GetLocalVector(_id, vector);
}

float
Body::mass()
{
	return b2Body_GetMass(_id);
}

b2MassData
Body::mass_data()
{
	return b2Body_GetMassData(_id);
}

b2MotionLocks
Body::motion_locks()
{
	return b2Body_GetMotionLocks(_id);
}

const char *
Body::name()
{
	return b2Body_GetName(_id);
}

b2Vec2
Body::position()
{
	return b2Body_GetPosition(_id);
}

b2Rot
Body::rotation()
{
	return b2Body_GetRotation(_id);
}

float
Body::rotational_inertia()
{
	return b2Body_GetRotationalInertia(_id);
}

void
Body::set_angular_damping(float value)
{
	return b2Body_SetAngularDamping(_id, value);
}

void
Body::set_angular_velocity(float value)
{
	return b2Body_SetAngularVelocity(_id, value);
}

void
Body::set_awake(bool value)
{
	b2Body_SetAwake(_id, value);
}

void
Body::set_bullet(bool value)
{
	b2Body_SetBullet(_id, value);
}

void
Body::set_gravity_scale(float value)
{
	b2Body_SetGravityScale(_id, value);
}

void
Body::set_linear_damping(float value)
{
	b2Body_SetLinearDamping(_id, value);
}

void
Body::set_linear_velocity(b2Vec2 v)
{
	b2Body_SetLinearVelocity(_id, v);
}

void
Body::set_mass_data(b2MassData md)
{
	b2Body_SetMassData(_id, md);
}

void
Body::set_motion_locks(b2MotionLocks locks)
{
	b2Body_SetMotionLocks(_id, locks);
}

void
Body::set_name(const char *name)
{
	b2Body_SetName(_id, name);
}

void
Body::set_sleep_threshold(float value)
{
	b2Body_SetSleepThreshold(_id, value);
}

void
Body::set_transform(b2Vec2 position, b2Rot rotation)
{
	b2Body_SetTransform(_id, position, rotation);
}

void
Body::set_type(b2BodyType type)
{
	b2Body_SetType(_id, type);
}

std::vector<euler::util::Reference<euler::physics::Shape>>
Body::shapes()
{
	std::vector<b2ShapeId> b2_shapes;
	const auto count = b2Body_GetShapeCount(_id);
	b2_shapes.resize(static_cast<size_t>(count));
	b2Body_GetShapes(_id, b2_shapes.data(), count);
	std::vector<util::Reference<Shape>> shapes;
	shapes.reserve(static_cast<size_t>(count));
	for (const auto &b2_shape : b2_shapes) {
		auto ptr = new Shape(b2_shape);
		auto shape = util::Reference(ptr);
		shapes.emplace_back(shape);
	}
	return shapes;
}

void
Body::set_sleep_enabled(const bool enabled)
{
	b2Body_EnableSleep(_id, enabled);
}

float
Body::sleep_threshold()
{
	return b2Body_GetSleepThreshold(_id);
}

void
Body::target_transform(b2Transform tform, float step, bool wake)
{
	b2Body_SetTargetTransform(_id, tform, step, wake);
}

b2Transform
Body::transform()
{
	return b2Body_GetTransform(_id);
}

b2BodyType
Body::type()
{
	return b2Body_GetType(_id);
}

void
Body::wake_touching()
{
	b2Body_WakeTouching(_id);
}

euler::util::Reference<euler::physics::World>
Body::world()
{
	const auto world = b2Body_GetWorld(_id);
	return World::wrap(world);
}

b2Vec2
Body::world_center_of_mass()
{
	return b2Body_GetWorldCenterOfMass(_id);
}

b2Vec2
Body::world_point(b2Vec2 point)
{
	return b2Body_GetWorldPoint(_id, point);
}

b2Vec2
Body::world_point_velocity(b2Vec2 point)
{
	return b2Body_GetWorldPointVelocity(_id, point);
}

b2Vec2
Body::world_vector(b2Vec2 vector)
{
	return b2Body_GetWorldVector(_id, vector);
}

euler::util::Reference<Body>
Body::unwrap(mrb_state *mrb, mrb_value self)
{
	const auto state = util::State::get(mrb);
	return euler::util::unwrap<Body>(state, self);
}