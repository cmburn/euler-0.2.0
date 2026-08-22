/* SPDX-License-Identifier: ISC */

#include <cassert>

#include "euler/physics/contact.h"

#include <mruby/class.h>

#include "euler/physics/shape.h"
#include "euler/physics/util.h"
#include "euler/physics/world.h"

using euler::physics::Contact;

/**
 * @overload Euler::Physics::Contact#valid?
 *   @return [Boolean] whether the contact is valid
 */
static mrb_value
contact_is_valid(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto contact = state->unwrap<Contact>(self);
	const bool valid = contact->is_valid();
	return mrb_bool_value(valid);
}

/**
 * @overload Euler::Physics::Contact#data
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
contact_data(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	// const auto contact = Contact::unwrap(mrb, self);
	const auto contact = state->unwrap<Contact>(self);
	auto data = contact->data();
	return data.wrap(mrb);
}
Contact::Data::~Data() = default;
Contact::Event::~Event() = default;
Contact::HitEvent::~HitEvent() = default;

mrb_value
Contact::HitEvent::wrap(mrb_state *mrb)
{
	const auto state = util::State::get(mrb);
	const mrb_value hash = state->rb().hash_new_capa(7);
	const auto shape_a_value = state->wrap(shape_a);
	state->rb().hash_set(hash, "shape_a", shape_a_value);
	const auto shape_b_value = state->wrap(shape_b);
	state->rb().hash_set(hash, "shape_b", shape_b_value);
	const auto contact_value = state->wrap(contact);
	state->rb().hash_set(hash, "contact", contact_value);
	state->rb().hash_set(hash, "point", b2_vec_to_value(mrb, point));
	state->rb().hash_set(hash, "normal", b2_vec_to_value(mrb, normal));
	state->rb().hash_set(hash, "approach_speed",
	    state->rb().float_value(approach_speed));
	return hash;
}

Contact::Events::~Events() = default;
Contact::~Contact() { world()->drop_contact(_id); }

euler::util::Reference<Contact>
Contact::wrap(const b2ContactId id)
{
	return world(id)->wrap_contact(id);
}

static mrb_value
manifold_point_to_value(mrb_state *mrb, const b2ManifoldPoint &mp)
{
	const auto state = euler::util::State::get(mrb);
	auto hash = state->rb().hash_new_capa(9);
	state->rb().hash_set(hash, "point",
	    euler::physics::b2_vec_to_value(mrb, mp.point));
	auto anchors = state->rb().ary_new_capa(2);
	state->rb().ary_push(anchors,
	    euler::physics::b2_vec_to_value(mrb, mp.anchorA));
	state->rb().ary_push(anchors,
	    euler::physics::b2_vec_to_value(mrb, mp.anchorB));
	state->rb().hash_set(hash, "anchors", anchors);
	state->rb().hash_set(hash, "separation",
	    state->rb().float_value(mp.separation));
	state->rb().hash_set(hash, "normal_impulse",
	    state->rb().float_value(mp.normalImpulse));
	state->rb().hash_set(hash, "tangent_impulse",
	    state->rb().float_value(mp.tangentImpulse));
	state->rb().hash_set(hash, "total_normal_impulse",
	    state->rb().float_value(mp.totalNormalImpulse));
	state->rb().hash_set(hash, "normal_velocity",
	    state->rb().float_value(mp.normalVelocity));
	state->rb().hash_set(hash, "id", mrb_fixnum_value(mp.id));
	state->rb().hash_set(hash, "persisted", mrb_bool_value(mp.persisted));
	return hash;
}

static mrb_value
manifold_to_value(mrb_state *mrb, const b2Manifold &manifold)
{
	const auto state = euler::util::State::get(mrb);
	mrb_value hash = state->rb().hash_new();
	state->rb().hash_set(hash, "normal",
	    euler::physics::b2_vec_to_value(mrb, manifold.normal));
	state->rb().hash_set(hash, "rolling_impulse",
	    state->rb().float_value(manifold.rollingImpulse));
	const auto points = state->rb().ary_new_capa(manifold.pointCount);
	for (int i = 0; i < manifold.pointCount; ++i) {
		const b2ManifoldPoint &mp = manifold.points[i];
		mrb_value mp_value = manifold_point_to_value(mrb, mp);
		state->rb().ary_push(points, mp_value);
	}
	state->rb().hash_set(hash, "points", points);
	return hash;
}

mrb_value
Contact::Data::wrap(mrb_state *mrb)
{
	const auto state = util::State::get(mrb);
	const mrb_value hash = state->rb().hash_new();
	// contact
	const auto contact_value = state->wrap(contact);
	state->rb().hash_set(hash, "contact", contact_value);
	const auto shape_a_value = state->wrap(shape_a);
	const auto shape_b_value = state->wrap(shape_b);
	mrb_value shapes_array = state->rb().ary_new_capa(2);
	state->rb().ary_push(shapes_array, shape_a_value);
	state->rb().ary_push(shapes_array, shape_b_value);
	state->rb().hash_set(hash, "shapes", shapes_array);
	const auto manifold_value = manifold_to_value(mrb, manifold);
	state->rb().hash_set(hash, "manifold", manifold_value);
	return hash;
}

Contact::Data
Contact::Data::from_b2(const b2ContactData &data)
{
	Data contact_data;
	contact_data.contact = Contact::wrap(data.contactId);
	contact_data.shape_a = Shape::wrap(data.shapeIdA);
	contact_data.shape_b = Shape::wrap(data.shapeIdB);
	contact_data.manifold = data.manifold;
	return contact_data;
}

mrb_value
Contact::Event::wrap(mrb_state *mrb)
{
	const auto state = util::State::get(mrb);
	const mrb_value hash = state->rb().hash_new_capa(3);
	const auto shape_a_value = state->wrap(shape_a);
	state->rb().hash_set(hash, "shape_a", shape_a_value);
	const auto shape_b_value = state->wrap(shape_b);
	state->rb().hash_set(hash, "shape_b", shape_b_value);
	const auto contact_value = state->wrap(contact);
	state->rb().hash_set(hash, "contact", contact_value);
	return hash;
}

Contact::Events
Contact::Events::from_b2(const b2ContactEvents &events)
{
	Events out;
	out.start_events.reserve(events.beginCount);
	for (int i = 0; i < events.beginCount; ++i) {
		const auto &event = events.beginEvents[i];
		const auto shape_a = Shape::wrap(event.shapeIdA);
		const auto shape_b = Shape::wrap(event.shapeIdB);
		const auto contact = Contact::wrap(event.contactId);
		out.start_events.emplace_back(Event {
		    .shape_a = shape_a,
		    .shape_b = shape_b,
		    .contact = contact,
		});
	}
	out.end_events.reserve(events.endCount);
	for (int i = 0; i < events.endCount; ++i) {
		const auto &event = events.endEvents[i];
		const auto shape_a = Shape::wrap(event.shapeIdA);
		const auto shape_b = Shape::wrap(event.shapeIdB);
		const auto contact = Contact::wrap(event.contactId);
		out.end_events.emplace_back(Event {
		    .shape_a = shape_a,
		    .shape_b = shape_b,
		    .contact = contact,
		});
	}
	out.hit_events.reserve(events.hitCount);
	for (int i = 0; i < events.hitCount; ++i) {
		const auto &event = events.hitEvents[i];
		const auto shape_a = Shape::wrap(event.shapeIdA);
		const auto shape_b = Shape::wrap(event.shapeIdB);
		const auto contact = Contact::wrap(event.contactId);
		out.hit_events.emplace_back(HitEvent {
		    .shape_a = shape_a,
		    .shape_b = shape_b,
		    .contact = contact,
		    .point = event.point,
		    .normal = event.normal,
		    .approach_speed = event.approachSpeed,
		});
	}
	return out;
}

mrb_value
Contact::Events::wrap(mrb_state *mrb)
{
	const auto state = util::State::get(mrb);
	const mrb_value begin_ary
	    = state->rb().ary_new_capa(start_events.size());
	for (auto &event : start_events) {
		const mrb_value event_value = event.wrap(mrb);
		state->rb().ary_push(begin_ary, event_value);
	}
	const mrb_value end_ary = state->rb().ary_new_capa(end_events.size());
	for (auto &event : end_events) {
		const mrb_value event_value = event.wrap(mrb);
		state->rb().ary_push(end_ary, event_value);
	}
	const mrb_value hit_ary = state->rb().ary_new_capa(hit_events.size());
	for (auto &event : hit_events) {
		const mrb_value event_value = event.wrap(mrb);
		state->rb().ary_push(hit_ary, event_value);
	}
	const mrb_value result = state->rb().hash_new();
	state->rb().hash_set(result, "begin", begin_ary);
	state->rb().hash_set(result, "end", end_ary);
	state->rb().hash_set(result, "hit", hit_ary);
	return result;
}

bool
Contact::is_valid() const
{
	return b2Contact_IsValid(_id);
}

Contact::Data
Contact::data() const
{
	const auto b2_data = b2Contact_GetData(_id);
	return Data::from_b2(b2_data);
}

euler::util::Reference<euler::physics::World>
Contact::world()
{
	return world(_id);
}

euler::util::Reference<euler::physics::World>
Contact::world(b2ContactId id)
{
	const auto data = b2Contact_GetData(id);
	/* I don't think shapes can collide from different worlds, but just to
	 * check */
	assert(data.shapeIdA.world0 == data.shapeIdB.world0);
	const auto world_id = b2Shape_GetWorld(data.shapeIdA);
	return World::wrap(world_id);
}

RClass *
Contact::init(const util::Reference<util::State> &state, RClass *mod, RClass *)
{
	const auto mrb = state->mrb();
	auto cls
	    = mrb->define_class_under(mod, "Contact", state->object_class());
	mrb->define_method(cls, "valid?", contact_is_valid, MRB_ARGS_REQ(0));
	mrb->define_method(cls, "data", contact_data, MRB_ARGS_REQ(0));
	return cls;
}