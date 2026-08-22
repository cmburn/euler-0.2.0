/* SPDX-License-Identifier: ISC */

#include <assert.h>

#include "euler/physics/world.h"

#include <box2d/box2d.h>
#include <box2d/types.h>

#include <utility>

#include "euler/physics/shape.h"
#include "euler/physics/util.h"

using euler::physics::World;

static b2WorldDef
parse_world_new_args(mrb_state *mrb)
{
	const auto state = euler::util::State::get(mrb);
	static const char *KW_NAMES[] = {
		"gravity",
		"restitution_threshold",
		"hit_event_threshold",
		"contact_hertz",
		"contact_damping_ratio",
		"contact_speed",
		"maximum_linear_speed",
		"enable_sleep",
		"enable_continuous",
		"enable_contact_softening",
	};
	enum {
		GRAVITY = 0,
		RESTITUTION_THRESHOLD,
		HIT_EVENT_THRESHOLD,
		CONTACT_HERTZ,
		CONTACT_DAMPING_RATIO,
		CONTACT_SPEED,
		MAXIMUM_LINEAR_SPEED,
		ENABLE_SLEEP,
		ENABLE_CONTINUOUS,
		ENABLE_CONTACT_SOFTENING,
		KW_COUNT = std::size(KW_NAMES),
	};
	mrb_sym kw_syms[KW_COUNT];
	for (size_t i = 0; i < KW_COUNT; ++i) {
		const size_t len = strlen(KW_NAMES[i]);
		kw_syms[i] = state->rb().intern_static(KW_NAMES[i], len);
	}
	mrb_value kw_values[KW_COUNT];
	euler::physics::init_kw_values(kw_values);
	mrb_kwargs kwargs = {
		.num = KW_COUNT,
		.required = 0,
		.table = kw_syms,
		.values = kw_values,
		.rest = nullptr,
	};
	state->rb().get_args(":", &kwargs);
	b2WorldDef world_def = b2DefaultWorldDef();
	assert(GRAVITY < KW_COUNT);
	if (!mrb_undef_p(kw_values[GRAVITY])) {
		world_def.gravity
		    = euler::physics::value_to_b2_vec(mrb, kw_values[GRAVITY]);
	}
	if (!mrb_undef_p(kw_values[RESTITUTION_THRESHOLD])) {
		world_def.restitutionThreshold = static_cast<float>(
		    mrb_float(kw_values[RESTITUTION_THRESHOLD]));
	}
	if (!mrb_undef_p(kw_values[HIT_EVENT_THRESHOLD])) {
		world_def.hitEventThreshold = static_cast<float>(
		    mrb_float(kw_values[HIT_EVENT_THRESHOLD]));
	}
	if (!mrb_undef_p(kw_values[CONTACT_HERTZ])) {
		world_def.contactHertz
		    = static_cast<float>(mrb_float(kw_values[CONTACT_HERTZ]));
	}
	if (!mrb_undef_p(kw_values[CONTACT_DAMPING_RATIO])) {
		world_def.contactDampingRatio = static_cast<float>(
		    mrb_float(kw_values[CONTACT_DAMPING_RATIO]));
	}
	if (!mrb_undef_p(kw_values[CONTACT_SPEED])) {
		world_def.contactSpeed
		    = static_cast<float>(mrb_float(kw_values[CONTACT_SPEED]));
	}
	if (!mrb_undef_p(kw_values[MAXIMUM_LINEAR_SPEED])) {
		world_def.maximumLinearSpeed = static_cast<float>(
		    mrb_float(kw_values[MAXIMUM_LINEAR_SPEED]));
	}
	if (!mrb_undef_p(kw_values[ENABLE_SLEEP]))
		world_def.enableSleep = mrb_bool(kw_values[ENABLE_SLEEP]);
	if (!mrb_undef_p(kw_values[ENABLE_CONTINUOUS])) {
		world_def.enableContinuous
		    = mrb_bool(kw_values[ENABLE_CONTINUOUS]);
	}
	if (!mrb_undef_p(kw_values[ENABLE_CONTACT_SOFTENING])) {
		world_def.enableContactSoftening
		    = mrb_bool(kw_values[ENABLE_CONTACT_SOFTENING]);
	}
	return world_def;
}

mrb_value
world_allocate(mrb_state *mrb, mrb_value)
{
	const auto state = euler::util::State::get(mrb);
	auto world = new World();
	auto ref = euler::util::Reference(world);
	auto out = state->wrap(ref);
	return out;
}

mrb_value
world_initialize(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const b2WorldDef def = parse_world_new_args(mrb);
	const auto world = state->unwrap<World>(self);
	world->initialize(def);
	return mrb_nil_value();
}

/**
 * Tests whether the world is valid according to Box2D.
 *
 * @return [Boolean] true if the world is valid, false otherwise.
 */
static mrb_value
world_is_valid(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	const bool valid = world->is_valid();
	return mrb_bool_value(valid);
}

/**
 * Simulates a world for one time step. This performs collision detection,
 * integration, and constraint solving.
 * @param time_step [Float] The amount of time to simulate.
 * @param substep_count [Integer] The number of sub-steps, increasing the
 *   sub-step count can increase accuracy.
 */
static mrb_value
world_step(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	static const char *KW_NAMES[] = {
		"substeps",
	};
	enum {
		SUBSTEPS = 0,
		KW_COUNT = sizeof(KW_NAMES) / sizeof(KW_NAMES[0]),
	};
	const auto world = state->unwrap<World>(self);
	mrb_float time_step = 1.0f / 60.0f;
	int substep_count = 4;
	mrb_sym kw_syms[KW_COUNT];
	for (size_t i = 0; i < KW_COUNT; ++i) {
		const size_t len = strlen(KW_NAMES[i]);
		kw_syms[i] = state->rb().intern_static(KW_NAMES[i], len);
	}
	mrb_value kw_values[KW_COUNT];
	euler::physics::init_kw_values(kw_values);
	const mrb_kwargs kwargs = {
		.num = KW_COUNT,
		.required = 0,
		.table = kw_syms,
		.values = kw_values,
		.rest = nullptr,
	};
	state->rb().get_args("|f:", &time_step, &kwargs);
	if (!mrb_undef_p(kw_values[SUBSTEPS])) {
		substep_count
		    = static_cast<int>(mrb_integer(kw_values[SUBSTEPS]));
	}
	world->step(static_cast<float>(time_step), substep_count);
	return mrb_nil_value();
}

/**
 * Get the body events for the current time step.
 *
 * Returned hashes have the following keys::
 * - :transform - [Euler::Physics::Transform] The new transform of the body.
 * - :body - [Euler::Physics::Body] The body that moved.
 * - :fell_asleep - [Boolean] true if the body fell asleep this step.

 * @return [Array<Hash>] An array of body move event hashes.
 */
static mrb_value
world_body_events(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	auto events = world->body_events();
	const mrb_value ary = state->rb().ary_new_capa(events.size());
	for (auto &event : events) state->rb().ary_push(ary, event.wrap(mrb));
	return ary;
}

/**
 *
 * @return [Hash] A hash with :begin and :end keys containing arrays of sensor
 *                touch event hashes.
 */
static mrb_value
world_sensor_events(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	return world->sensor_events().wrap(mrb);
}

// static mrb_value
// contact_bound_event_to_hash(mrb_state *mrb, const box2d_world *world,
//     const b2ShapeId shape_a, const b2ShapeId shape_b)
// {
// 	const auto state = euler::util::State::get(mrb);
// 	const mrb_value hash = state->rb().hash_new();
// 	// const mrb_value a = box2d_shape_wrap(mrb, shape_a);
// 	// const mrb_value b = box2d_shape_wrap(mrb, shape_b);
// 	state->rb().hash_set(hash, EULER_SYM_VAL(a), a);
// 	state->rb().hash_set(hash, EULER_SYM_VAL(b), b);
// 	return hash;
// }

static mrb_value
world_contact_events(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	return world->contact_events().wrap(mrb);
}

static mrb_value
world_joint_events(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	return world->joint_events().wrap(mrb);
}

/* TODO: arg cleanup */
static mrb_value
world_overlap_aabb(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	static const char *KW_NAMES[] = {
		"lower_bound",
		"upper_bound",
	};
	enum {
		LOWER_BOUND = 0,
		UPPER_BOUND,
		KW_COUNT = sizeof(KW_NAMES) / sizeof(KW_NAMES[0]),
	};
	mrb_sym kw_syms[KW_COUNT];
	for (size_t i = 0; i < KW_COUNT; ++i) {
		const size_t len = strlen(KW_NAMES[i]);
		kw_syms[i] = state->rb().intern_static(KW_NAMES[i], len);
	}
	mrb_value kw_values[KW_COUNT];
	euler::physics::init_kw_values(kw_values);
	const mrb_kwargs kwargs = {
		.num = KW_COUNT,
		.required = KW_COUNT,
		.table = kw_syms,
		.values = kw_values,
		.rest = nullptr,
	};
	mrb_value block;
	state->rb().get_args("&:", &block, &kwargs);
	const b2Vec2 lower_bound
	    = euler::physics::value_to_b2_vec(mrb, kw_values[LOWER_BOUND]);
	const b2Vec2 upper_bound
	    = euler::physics::value_to_b2_vec(mrb, kw_values[UPPER_BOUND]);
	const b2AABB aabb = {
		.lowerBound = lower_bound,
		.upperBound = upper_bound,
	};
	mrb_value result = mrb_nil_value();
	if (!mrb_nil_p(block)) result = state->rb().ary_new();
	world->overlap_aabb(aabb, [&](auto &shape) {
		if (mrb_nil_p(block)) {
			state->rb().ary_push(result, state->wrap(shape));
			return true;
		}
		const auto yield_result
		    = state->rb().yield(block, state->wrap(shape));
		return mrb_true_p(yield_result);
	});
	return result;
}

static b2ShapeProxy
read_shape_proxy(mrb_state *mrb, const mrb_value value)
{
	const auto state = euler::util::State::get(mrb);
	/* should be a hash with two keys: points and radius */
	if (!mrb_hash_p(value))
		state->rb().raise(state->rb().type_error(),
		    "Expected hash for shape proxy");
	const mrb_value points_value
	    = state->rb().hash_get(value, EULER_SYM_VAL(points));
	if (!mrb_array_p(points_value)) {
		state->rb().raise(state->rb().type_error(),
		    "Expected array for shape proxy points");
	}
	if (RARRAY_LEN(points_value) == 0) {
		state->rb().raise(state->rb().argument_error(),
		    "Shape proxy points array cannot be empty");
	}
	const int n_points = static_cast<int>(RARRAY_LEN(points_value));
	if (n_points > B2_MAX_POLYGON_VERTICES) {
		state->rb().raise(state->rb().argument_error(),
		    "Shape proxy points array exceeds maximum vertices");
	}
	b2Vec2 points[B2_MAX_POLYGON_VERTICES] = {};
	for (int i = 0; i < n_points; ++i) {
		const auto point_value = state->rb().ary_ref(points_value, i);
		points[i] = euler::physics::value_to_b2_vec(mrb, point_value);
	}
	// radius is optional, defaults to 0
	float radius = 0.0f;
	if (state->rb().hash_key_p(value, EULER_SYM_VAL(radius))) {
		const mrb_value radius_value
		    = state->rb().hash_get(value, EULER_SYM_VAL(radius));
		radius = static_cast<float>(mrb_float(radius_value));
	}
	return b2MakeProxy(points, n_points, radius);
}

static mrb_value
world_overlap_shape(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	/*
	 * three scenarios for arguments (ignoring block):
	 * 1. Array of shape proxy hashes
	 * 2. Single shape proxy hash
	 * 3. Arguments of shape proxy hash directly as keywords
	 */
	const auto world = state->unwrap<World>(self);
	const mrb_value block = mrb_nil_value();
	static const char *KW_NAMES[] = {
		"points",
		"radius",
	};
	enum {
		POINTS = 0,
		RADIUS,
		KW_COUNT = std::size(KW_NAMES),
	};
	mrb_sym kw_syms[KW_COUNT];
	for (size_t i = 0; i < KW_COUNT; ++i) {
		const size_t len = strlen(KW_NAMES[i]);
		kw_syms[i] = state->rb().intern_static(KW_NAMES[i], len);
	}
	mrb_value kw_values[KW_COUNT];
	euler::physics::init_kw_values(kw_values);
	const mrb_kwargs kwargs = {
		.num = KW_COUNT,
		.required = 1,
		.table = kw_syms,
		.values = kw_values,
		.rest = nullptr,
	};
	state->rb().get_args("&:", &block, &kwargs);
	mrb_value hash = state->rb().hash_new();
	mrb_value points = kw_values[POINTS];
	if (!mrb_array_p(points)) {
		state->rb().raise(state->rb().type_error(),
		    "Expected array for shape proxy points");
	}
	if (RARRAY_LEN(points) == 0) {
		state->rb().raise(state->rb().argument_error(),
		    "Shape proxy points array cannot be empty");
	}
	if (RARRAY_LEN(points) > B2_MAX_POLYGON_VERTICES) {
		state->rb().raisef(state->rb().argument_error(),
		    "Shape proxy points array exceeds maximum vertices (%d)",
		    B2_MAX_POLYGON_VERTICES);
	}
	state->rb().hash_set(hash, EULER_SYM_VAL(points), kw_values[POINTS]);
	if (!mrb_undef_p(kw_values[RADIUS]))
		state->rb().hash_set(hash, EULER_SYM_VAL(radius),
		    kw_values[RADIUS]);
	const b2ShapeProxy sp = read_shape_proxy(mrb, hash);
	auto result = mrb_nil_value();
	if (!mrb_nil_p(block)) result = state->rb().ary_new();
	world->overlap_shape(sp, [&](auto &shape) {
		if (mrb_nil_p(block)) {
			state->rb().ary_push(result, state->wrap(shape));
			return true;
		}
		const auto yield_result
		    = state->rb().yield(block, state->wrap(shape));
		state->rb().ary_push(result, yield_result);
		return true;
	});
	return result;
}

static void
read_cast_ray_args(mrb_state *mrb, b2Vec2 *origin, b2Vec2 *translation,
    b2QueryFilter *filter, mrb_value *block)
{
	const auto state = euler::util::State::get(mrb);
	static const char *KW_NAMES[] = {
		"origin",
		"translation",
	};
	enum {
		ORIGIN = 0,
		TRANSLATION,
		KW_COUNT = sizeof(KW_NAMES) / sizeof(KW_NAMES[0]),
	};
	mrb_sym kw_syms[KW_COUNT];
	for (size_t i = 0; i < KW_COUNT; ++i) {
		const size_t len = strlen(KW_NAMES[i]);
		kw_syms[i] = state->rb().intern_static(KW_NAMES[i], len);
	}
	mrb_value kw_values[KW_COUNT];
	euler::physics::init_kw_values(kw_values);
	const mrb_kwargs kwargs = {
		.num = KW_COUNT,
		.required = KW_COUNT,
		.table = kw_syms,
		.values = kw_values,
		.rest = nullptr,
	};
	if (block == nullptr) state->rb().get_args(":", &kwargs);
	else state->rb().get_args("&:", block, &kwargs);
	*origin = euler::physics::value_to_b2_vec(mrb, kw_values[ORIGIN]);
	*translation
	    = euler::physics::value_to_b2_vec(mrb, kw_values[TRANSLATION]);
	*filter = b2DefaultQueryFilter();
}

/* If no block is given, returns all shapes. If a block is given, the behavior
 * is determined by the block.
 * return -1: ignore this shape and continue
 * return 0, nil: terminate the ray cast
 * return fraction: clip the ray to this point
 * return 1: don't clip the ray and continue */
static mrb_value
world_cast_ray(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	b2Vec2 origin, translation;
	b2QueryFilter filter;
	mrb_value block = mrb_nil_value();
	read_cast_ray_args(mrb, &origin, &translation, &filter, &block);
	// result_context ctx = {
	// 	.mrb = mrb,
	// 	.world = world,
	// 	.block = block,
	// 	.result
	// 	= mrb_nil_p(block) ? state->rb().ary_new() : mrb_nil_value(),
	// };
	// world->CastRay(origin, translation, filter, cast_result_fn, &ctx);
	// return ctx.result;
	auto result = mrb_nil_value();
	if (!mrb_nil_p(block)) result = state->rb().ary_new();
	const auto fn = [&](auto &shape, auto point, auto normal,
			    auto fraction) -> float {
		if (mrb_nil_p(block)) {
			state->rb().ary_push(result, state->wrap(shape));
			return 1.0f;
		}
		auto block_result
		    = state->rb().call(block, state->wrap(shape),
			euler::physics::b2_vec_to_value(mrb, point),
			euler::physics::b2_vec_to_value(mrb, normal),
			state->rb().float_value(fraction));
		if (mrb_nil_p(block_result)) return 0.0f;
		const mrb_value float_value
		    = state->rb().ensure_float_type(block_result);
		return mrb_float(float_value);
	};
	(void)world->cast_ray(origin, translation, fn);
	return result;
}

static mrb_value
world_cast_ray_closest(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	b2Vec2 origin, translation;
	b2QueryFilter filter;
	read_cast_ray_args(mrb, &origin, &translation, &filter, nullptr);
	auto result = world->cast_ray_closest(origin, translation, filter);
	mrb_value shape = mrb_nil_value();
	if (B2_IS_NON_NULL(result.shapeId)) {
		auto ref = euler::physics::Shape::wrap(result.shapeId);
		shape = state->wrap(ref);
	}
	const mrb_value out = state->rb().hash_new_capa(5);
	state->rb().hash_set(out, EULER_SYM_VAL(shape), shape);
	state->rb().hash_set(out, EULER_SYM_VAL(point),
	    euler::physics::b2_vec_to_value(mrb, result.point));
	state->rb().hash_set(out, EULER_SYM_VAL(normal),
	    euler::physics::b2_vec_to_value(mrb, result.normal));
	state->rb().hash_set(out, EULER_SYM_VAL(fraction),
	    state->rb().float_value(result.fraction));
	state->rb().hash_set(out, EULER_SYM_VAL(hit),
	    mrb_bool_value(result.hit));
	return out;
}

static mrb_value
world_cast_shape(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_value block = mrb_nil_value();
	static const char *KW_NAMES[] = {
		"shape",
		"translation",
	};
	enum {
		SHAPE = 0,
		TRANSLATION,
		KW_COUNT = sizeof(KW_NAMES) / sizeof(KW_NAMES[0]),
	};
	mrb_sym kw_syms[KW_COUNT];
	for (size_t i = 0; i < KW_COUNT; ++i) {
		const size_t len = strlen(KW_NAMES[i]);
		kw_syms[i] = state->rb().intern_static(KW_NAMES[i], len);
	}
	mrb_value kw_values[KW_COUNT];
	euler::physics::init_kw_values(kw_values);
	const mrb_kwargs kwargs = {
		.num = KW_COUNT,
		.required = KW_COUNT,
		.table = kw_syms,
		.values = kw_values,
		.rest = nullptr,
	};
	state->rb().get_args("&:", &block, &kwargs);
	const b2ShapeProxy sp = read_shape_proxy(mrb, kw_values[SHAPE]);
	const b2Vec2 translation
	    = euler::physics::value_to_b2_vec(mrb, kw_values[TRANSLATION]);
	auto result = mrb_nil_value();
	if (!mrb_nil_p(block)) result = state->rb().ary_new();
	const auto fn = [&](auto &shape, auto point, auto normal,
			    auto fraction) -> float {
		if (mrb_nil_p(block)) {
			state->rb().ary_push(result, state->wrap(shape));
			return 1.0f;
		}
		auto block_result = state->rb().funcall(block, "call", 4,
		    state->wrap(shape),
		    euler::physics::b2_vec_to_value(mrb, point),
		    euler::physics::b2_vec_to_value(mrb, normal),
		    state->rb().float_value(fraction));
		if (mrb_nil_p(block_result)) return 0.0f;
		const mrb_value float_value
		    = state->rb().ensure_float_type(block_result);
		return mrb_float(float_value);
	};
	(void)world->cast_shape(sp, translation, fn);
	return result;
}

static b2Capsule
read_capsule_value(mrb_state *mrb, const mrb_value value)
{
	const auto state = euler::util::State::get(mrb);
	if (!mrb_hash_p(value)) {
		state->rb().raise(state->rb().type_error(),
		    "Expected hash for capsule");
	}

	const mrb_value points
	    = state->rb().hash_get(value, EULER_SYM_VAL(points));
	if (!mrb_array_p(points)) {
		state->rb().raise(state->rb().type_error(),
		    "Expected array for capsule points");
	}
	if (RARRAY_LEN(points) != 2) {
		state->rb().raise(state->rb().argument_error(),
		    "Expected array of length 2 for capsule points");
	}
	const b2Vec2 center1 = euler::physics::value_to_b2_vec(mrb,
	    state->rb().ary_ref(points, 0));
	const b2Vec2 center2 = euler::physics::value_to_b2_vec(mrb,
	    state->rb().ary_ref(points, 1));
	const mrb_value radius_value
	    = state->rb().hash_get(value, EULER_SYM_VAL(radius));
	if (!mrb_float_p(radius_value) && !mrb_integer_p(radius_value)) {
		state->rb().raise(state->rb().type_error(),
		    "Expected float for capsule radius");
	}
	const float radius = static_cast<float>(mrb_float(radius_value));
	return b2Capsule {
		.center1 = center1,
		.center2 = center2,
		.radius = radius,
	};
}

static mrb_value
world_cast_mover(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_value block = mrb_nil_value();
	static const char *KW_NAMES[] = {
		"mover",
		"translation",
	};
	enum {
		MOVER = 0,
		TRANSLATION,
		KW_COUNT = sizeof(KW_NAMES) / sizeof(KW_NAMES[0]),
	};
	mrb_sym kw_syms[KW_COUNT];
	for (size_t i = 0; i < KW_COUNT; ++i) {
		const size_t len = strlen(KW_NAMES[i]);
		kw_syms[i] = state->rb().intern_static(KW_NAMES[i], len);
	}
	mrb_value kw_values[KW_COUNT];
	euler::physics::init_kw_values(kw_values);
	const mrb_kwargs kwargs = {
		.num = KW_COUNT,
		.required = KW_COUNT,
		.table = kw_syms,
		.values = kw_values,
		.rest = nullptr,
	};
	state->rb().get_args("&:", &block, &kwargs);
	const b2Capsule mover = read_capsule_value(mrb, kw_values[MOVER]);
	const b2Vec2 translation
	    = euler::physics::value_to_b2_vec(mrb, kw_values[TRANSLATION]);
	const b2QueryFilter filter = b2DefaultQueryFilter();
	const auto out = world->cast_mover(&mover, translation, filter);
	return state->rb().float_value(out);
}

static mrb_value
world_collide_mover(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_value block = mrb_nil_value();
	mrb_value mover_value = mrb_nil_value();
	state->rb().get_args("H&", &mover_value, &block);
	const b2Capsule mover = read_capsule_value(mrb, mover_value);
	auto result = mrb_nil_value();
	if (!mrb_nil_p(block)) result = state->rb().ary_new();
	world->collide_mover(&mover, [&](auto shape, auto plane) {
		if (mrb_nil_p(block)) {
			state->rb().ary_push(result, state->wrap(shape));
			return true;
		}
		const mrb_value plane_result = state->rb().hash_new_capa(3);
		const mrb_value col_plane = state->rb().hash_new_capa(2);
		state->rb().hash_set(col_plane, EULER_SYM_VAL(normal),
		    euler::physics::b2_vec_to_value(mrb, plane->plane.normal));
		state->rb().hash_set(col_plane, EULER_SYM_VAL(offset),
		    state->rb().float_value(plane->plane.offset));
		state->rb().hash_set(plane_result, EULER_SYM_VAL(plane),
		    col_plane);
		state->rb().hash_set(plane_result, EULER_SYM_VAL(point),
		    euler::physics::b2_vec_to_value(mrb, plane->point));
		state->rb().hash_set(plane_result, EULER_SYM_VAL(hit),
		    mrb_bool_value(plane->hit));
		// mrb_value args[2] = { state->wrap(shape), plane_result };
		auto ans = state->rb().call(block, state->wrap(shape),
		    plane_result);
		return mrb_true_p(ans);
	});
	return result;
}

static mrb_value
world_enable_sleeping(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_bool value;
	state->rb().get_args("b", &value);
	world->enable_sleeping(value);
	return mrb_nil_value();
}

static mrb_value
world_is_sleeping_enabled(mrb_state *mrb, const mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	const bool enabled = world->is_sleeping_enabled();
	return mrb_bool_value(enabled);
}

static mrb_value
world_enable_continuous(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_bool value;
	state->rb().get_args("b", &value);
	world->enable_continuous(value);
	return mrb_nil_value();
}

static mrb_value
world_is_continuous_enabled(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	const bool enabled = world->is_continuous_enabled();
	return mrb_bool_value(enabled);
}

static mrb_value
world_set_restitution_threshold(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_float value;
	state->rb().get_args("f", &value);
	world->set_restitution_threshold(static_cast<float>(value));
	return mrb_nil_value();
}

static mrb_value
world_restitution_threshold(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	const float threshold = world->restitution_threshold();
	return state->rb().float_value(threshold);
}

static mrb_value
world_set_hit_event_threshold(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_float value;
	state->rb().get_args("f", &value);
	world->set_hit_event_threshold(static_cast<float>(value));
	return mrb_nil_value();
}

static mrb_value
world_hit_event_threshold(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	const float threshold = world->hit_event_threshold();
	return state->rb().float_value(threshold);
}

static mrb_value
world_set_custom_filter(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_value block;
	state->rb().get_args("&!", &block);
	world->set_custom_filter(mrb, block);
	return mrb_nil_value();
}

static mrb_value
world_on_pre_solve(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_value block;
	state->rb().get_args("&!", &block);
	world->set_pre_solve(mrb, block);
	return mrb_nil_value();
}

static mrb_value
world_set_gravity(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_value value;
	state->rb().get_args("H", &value);
	const b2Vec2 gravity = euler::physics::value_to_b2_vec(mrb, value);
	world->set_gravity(gravity);
	return mrb_nil_value();
}

static mrb_value
world_gravity(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	const b2Vec2 gravity = world->gravity();
	return euler::physics::b2_vec_to_value(mrb, gravity);
}

static mrb_value
world_explode(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	static const char *KW_NAMES[] = {
		"position",
		"radius",
		"falloff",
		"impulse_per_length",
	};
	enum {
		POSITION = 0,
		RADIUS,
		FALLOFF,
		IMPULSE_PER_LENGTH,
		KW_COUNT = std::size(KW_NAMES),
	};
	mrb_sym kw_syms[KW_COUNT];
	for (size_t i = 0; i < KW_COUNT; ++i) {
		const size_t len = strlen(KW_NAMES[i]);
		kw_syms[i] = state->rb().intern_static(KW_NAMES[i], len);
	}
	mrb_value kw_values[KW_COUNT];
	euler::physics::init_kw_values(kw_values);
	const mrb_kwargs kwargs = {
		.num = KW_COUNT,
		.required = 0,
		.table = kw_syms,
		.values = kw_values,
		.rest = nullptr,
	};
	state->rb().get_args(":", &kwargs);
	b2ExplosionDef def = b2DefaultExplosionDef();
	if (!mrb_undef_p(kw_values[POSITION])) {
		def.position
		    = euler::physics::value_to_b2_vec(mrb, kw_values[POSITION]);
	}
	if (!mrb_undef_p(kw_values[RADIUS]))
		def.radius = static_cast<float>(mrb_float(kw_values[RADIUS]));
	if (!mrb_undef_p(kw_values[FALLOFF]))
		def.falloff = static_cast<float>(mrb_float(kw_values[FALLOFF]));
	if (!mrb_undef_p(kw_values[IMPULSE_PER_LENGTH])) {
		def.impulsePerLength = static_cast<float>(
		    mrb_float(kw_values[IMPULSE_PER_LENGTH]));
	}
	world->explode(def);
	return mrb_nil_value();
}

static mrb_value
world_set_contact_tuning(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_value hash;
	state->rb().get_args("H", &hash);
	float hertz = 0.0f;
	float damping_ratio = 0.0f;
	float push_speed = 0.0f;
	const std::array keys = {
		std::pair(EULER_SYM_VAL(hertz), &hertz),
		std::pair(EULER_SYM_VAL(damping_ratio), &damping_ratio),
		std::pair(EULER_SYM_VAL(push_speed), &push_speed),
	};
	for (const auto &key : keys) {
		if (!state->rb().hash_key_p(hash, key.first)) {
			const auto sym = mrb_symbol(key.first);
			const char *key_name = state->rb().sym_name(sym);
			state->rb().raisef(state->rb().argument_error(),
			    "Missing required key %s in contact_tuning hash",
			    key_name);
		}
		const auto value = state->rb().hash_get(hash, key.first);
		*(key.second) = static_cast<float>(mrb_float(value));
	}
	world->set_contact_tuning(hertz, damping_ratio, push_speed);
	return mrb_nil_value();
}

static mrb_value
world_set_maximum_linear_speed(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	mrb_float value;
	state->rb().get_args("f", &value);
	world->set_maximum_linear_speed(static_cast<float>(value));
	return mrb_nil_value();
}

static mrb_value
world_maximum_linear_speed(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	const float speed = world->maximum_linear_speed();
	return state->rb().float_value(speed);
}

static mrb_value
world_awake_body_count(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	const int count = world->awake_body_count();
	return state->rb().int_value(count);
}

static b2MotionLocks
read_motion_locks_args(mrb_state *mrb, mrb_value locks_value)
{
	const auto state = euler::util::State::get(mrb);
	if (!mrb_array_p(locks_value)) {
		state->rb().raise(state->rb().type_error(),
		    "Expected array of locked directions for "
		    "motion_locks");
	}
	bool x = false, y = false, z = false;
	for (mrb_int i = 0; i < RARRAY_LEN(locks_value); ++i) {
		const mrb_value a = state->rb().ary_ref(locks_value, i);
		if (!mrb_symbol_p(a)) {
			state->rb().raise(state->rb().type_error(),
			    "Expected symbols in motion_locks array");
		}
		if (state->rb().equal(a, EULER_SYM_VAL(linear_x))) x = true;
		if (state->rb().equal(a, EULER_SYM_VAL(x))) x = true;
		if (state->rb().equal(a, EULER_SYM_VAL(linear_y))) y = true;
		if (state->rb().equal(a, EULER_SYM_VAL(y))) y = true;
		if (state->rb().equal(a, EULER_SYM_VAL(angular_z))) z = true;
		if (state->rb().equal(a, EULER_SYM_VAL(z))) z = true;
	}
	return b2MotionLocks {
		.linearX = x,
		.linearY = y,
		.angularZ = z,
	};
}

static mrb_value
world_create_body(mrb_state *mrb, mrb_value self)
{
	const auto state = euler::util::State::get(mrb);
	const auto world = state->unwrap<World>(self);
	static const char *KW_NAMES[] = {
		"type",
		"rotation",
		"angle",
		"linear_velocity",
		"angular_velocity",
		"linear_damping",
		"angular_damping",
		"gravity_scale",
		"sleep_threshold",
		"motion_locks",
		"enable_sleep",
		"awake",
		"bullet",
		"enabled",
		"allow_fast_rotation",
	};
	enum {
		TYPE = 0,
		POSITION,
		ROTATION,
		LINEAR_VELOCITY,
		ANGULAR_VELOCITY,
		LINEAR_DAMPING,
		ANGULAR_DAMPING,
		GRAVITY_SCALE,
		SLEEP_THRESHOLD,
		MOTION_LOCKS,
		ENABLE_SLEEP,
		AWAKE,
		BULLET,
		ENABLED,
		ALLOW_FAST_ROTATION,
		KW_COUNT = std::size(KW_NAMES),
	};
	mrb_sym kw_syms[KW_COUNT];
	for (size_t i = 0; i < KW_COUNT; ++i) {
		const size_t len = strlen(KW_NAMES[i]);
		kw_syms[i] = state->rb().intern_static(KW_NAMES[i], len);
	}
	mrb_value kw_values[KW_COUNT];
	euler::physics::init_kw_values(kw_values);
	const mrb_kwargs kwargs = {
		.num = KW_COUNT,
		.required = 0,
		.table = kw_syms,
		.values = kw_values,
		.rest = nullptr,
	};
	state->rb().get_args(":", &kwargs);
	b2BodyDef def = b2DefaultBodyDef();
	if (!mrb_nil_p(kw_values[TYPE])) {
		def.type
		    = euler::physics::Body::parse_type(mrb, kw_values[TYPE]);
	}
	if (!mrb_nil_p(kw_values[POSITION])) {
		def.position
		    = euler::physics::value_to_b2_vec(mrb, kw_values[POSITION]);
	}
	if (!mrb_nil_p(kw_values[TYPE]))
		def.rotation
		    = euler::physics::value_to_b2_rot(mrb, kw_values[ROTATION]);
	if (!mrb_nil_p(kw_values[LINEAR_VELOCITY])) {
		def.linearVelocity = euler::physics::value_to_b2_vec(mrb,
		    kw_values[LINEAR_VELOCITY]);
	}
	if (!mrb_nil_p(kw_values[ANGULAR_VELOCITY])) {
		def.angularVelocity = static_cast<float>(
		    mrb_float(kw_values[ANGULAR_VELOCITY]));
	}
	if (!mrb_nil_p(kw_values[LINEAR_DAMPING])) {
		def.linearDamping
		    = static_cast<float>(mrb_float(kw_values[LINEAR_DAMPING]));
	}
	if (!mrb_nil_p(kw_values[ANGULAR_DAMPING])) {
		def.angularDamping
		    = static_cast<float>(mrb_float(kw_values[ANGULAR_DAMPING]));
	}
	if (!mrb_nil_p(kw_values[GRAVITY_SCALE])) {
		def.gravityScale
		    = static_cast<float>(mrb_float(kw_values[GRAVITY_SCALE]));
	}
	if (!mrb_nil_p(kw_values[SLEEP_THRESHOLD])) {
		def.sleepThreshold
		    = static_cast<float>(mrb_float(kw_values[SLEEP_THRESHOLD]));
	}
	if (!mrb_nil_p(kw_values[MOTION_LOCKS])) {
		// expect an array of symbols
		const mrb_value locks_value = kw_values[MOTION_LOCKS];
		def.motionLocks = read_motion_locks_args(mrb, locks_value);
	}
	if (!mrb_nil_p(kw_values[ENABLE_SLEEP])) {
		def.enableSleep = mrb_bool(kw_values[ENABLE_SLEEP]);
	}
	if (!mrb_nil_p(kw_values[AWAKE])) {
		def.isAwake = mrb_bool(kw_values[AWAKE]);
	}
	if (!mrb_nil_p(kw_values[BULLET])) {
		def.isBullet = mrb_bool(kw_values[BULLET]);
	}
	if (!mrb_nil_p(kw_values[ENABLED])) {
		def.isEnabled = mrb_bool(kw_values[ENABLED]);
	}
	if (!mrb_nil_p(kw_values[ALLOW_FAST_ROTATION])) {
		def.allowFastRotation
		    = mrb_bool(kw_values[ALLOW_FAST_ROTATION]);
	}
	auto body = world->create_body(def);
	return state->wrap(body);
}

RClass *
box2d_world_init(mrb_state *mrb, RClass *mod)
{
	const auto state = euler::util::State::get(mrb);
	RClass *world
	    = state->rb().define_class_under(mod, "World", mrb->object_class);
	MRB_SET_INSTANCE_TT(world, MRB_TT_DATA);
	state->rb().define_class_method(world, "allocate", world_allocate,
	    MRB_ARGS_NONE());
	state->rb().define_method(world, "initialize", world_initialize,
	    MRB_ARGS_NONE() | MRB_ARGS_KEY(0, 0));
	state->rb().define_method(world, "valid?", world_is_valid,
	    MRB_ARGS_NONE());
	state->rb().define_method(world, "step", world_step,
	    MRB_ARGS_KEY(2, 0));
	state->rb().define_method(world, "body_events", world_body_events,
	    MRB_ARGS_NONE());
	state->rb().define_method(world, "sensor_events", world_sensor_events,
	    MRB_ARGS_NONE());
	state->rb().define_method(world, "contact_events",
	    world_contact_events, MRB_ARGS_NONE());
	state->rb().define_method(world, "joint_events", world_joint_events,
	    MRB_ARGS_NONE());
	state->rb().define_method(world, "overlap_aabb", world_overlap_aabb,
	    MRB_ARGS_KEY(2, 0));
	state->rb().define_method(world, "overlap_shape", world_overlap_shape,
	    MRB_ARGS_KEY(2, 0));
	state->rb().define_method(world, "cast_ray", world_cast_ray,
	    MRB_ARGS_KEY(2, 0));
	state->rb().define_method(world, "cast_ray_closest",
	    world_cast_ray_closest, MRB_ARGS_KEY(2, 0));
	state->rb().define_method(world, "cast_shape", world_cast_shape,
	    MRB_ARGS_KEY(2, 0));
	state->rb().define_method(world, "cast_mover", world_cast_mover,
	    MRB_ARGS_KEY(2, 0));
	state->rb().define_method(world, "collide_mover", world_collide_mover,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(world, "enable_sleeping",
	    world_enable_sleeping, MRB_ARGS_REQ(1));
	state->rb().define_method(world, "sleeping_enabled?",
	    world_is_sleeping_enabled, MRB_ARGS_NONE());
	state->rb().define_method(world,
	    "continuous_enabled=", world_enable_continuous, MRB_ARGS_REQ(1));
	state->rb().define_method(world, "continuous_enabled",
	    world_is_continuous_enabled, MRB_ARGS_NONE());
	state->rb().alias_method(world,
	    state->rb().intern_cstr("enable_continuous"),
	    state->rb().intern_cstr("continuous_enabled="));
	state->rb().define_method(world,
	    "restitution_threshold=", world_set_restitution_threshold,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(world, "restitution_threshold",
	    world_restitution_threshold, MRB_ARGS_NONE());
	state->rb().define_method(world,
	    "hit_event_threshold=", world_set_hit_event_threshold,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(world, "hit_event_threshold",
	    world_hit_event_threshold, MRB_ARGS_NONE());
	state->rb().define_method(world, "on_custom_filter",
	    world_set_custom_filter, MRB_ARGS_BLOCK());
	state->rb().define_method(world, "on_pre_solve", world_on_pre_solve,
	    MRB_ARGS_BLOCK());
	state->rb().define_method(world, "gravity=", world_set_gravity,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(world, "gravity", world_gravity,
	    MRB_ARGS_NONE());
	state->rb().define_method(world, "explode", world_explode,
	    MRB_ARGS_KEY(4, 0));
	state->rb().define_method(world,
	    "contact_tuning=", world_set_contact_tuning, MRB_ARGS_REQ(1));
	state->rb().define_method(world,
	    "maximum_linear_speed=", world_set_maximum_linear_speed,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(world, "maximum_linear_speed",
	    world_maximum_linear_speed, MRB_ARGS_NONE());
	state->rb().define_method(world, "awake_body_count",
	    world_awake_body_count, MRB_ARGS_NONE());
	state->rb().define_method(world, "create_body", world_create_body,
	    MRB_ARGS_KEY(15, 0));
	return world;
}

RClass *
World::init(const util::Reference<util::State> &state, RClass *mod, RClass *)
{
	return box2d_world_init(state->rb().mrb(), mod);
}

mrb_value
World::wrap(const util::Reference<util::State> &state)
{
	auto self = util::Reference(this);
	return state->wrap<World>(self);
}

bool
World::overlap_result_fn(b2ShapeId shape_id, void *user_data)
{
	auto shape = Shape::wrap(shape_id);
	const auto fn = static_cast<OverlapFn *>(user_data);
	return (*fn)(shape);
}
float
World::cast_result_fn(b2ShapeId shape_id, b2Vec2 point, b2Vec2 normal,
    float fraction, void *context)
{
	auto shape = Shape::wrap(shape_id);
	const auto fn = static_cast<CastResultFn *>(context);
	return (*fn)(shape, point, normal, fraction);
}
bool
World::plane_result_fn(const b2ShapeId shape_id, const b2PlaneResult *plane,
    void *context)
{
	auto shape = Shape::wrap(shape_id);
	const auto fn = static_cast<PlaneResultFn *>(context);
	return (*fn)(shape, plane);
}
bool
World::custom_filter_fn(b2ShapeId shape_a_id, b2ShapeId shape_b_id,
    void *context)
{
	auto shape_a = Shape::wrap(shape_a_id);
	auto shape_b = Shape::wrap(shape_b_id);
	const auto fn = static_cast<CustomFilterFn *>(context);
	return (*fn)(shape_a, shape_b);
}
bool
World::pre_solve_fn(b2ShapeId shape_a_id, b2ShapeId shape_b_id, b2Vec2 point,
    b2Vec2 normal, void *context)
{
	auto shape_a = Shape::wrap(shape_a_id);
	auto shape_b = Shape::wrap(shape_b_id);
	const auto fn = static_cast<PreSolveFn *>(context);
	return (*fn)(shape_a, shape_b, point, normal);
}
void
World::initialize(const b2WorldDef &def)
{
	assert(B2_IS_NULL(_id));
	_id = b2CreateWorld(&def);
	b2World_SetUserData(_id, this);
}

euler::util::Reference<World>
World::wrap(b2WorldId id)
{
	auto user_data = b2World_GetUserData(id);
	if (user_data != nullptr) return { static_cast<World *>(user_data) };
	auto world = new World(id);
	b2World_SetUserData(id, world);
	return { world };
}

euler::util::Reference<euler::util::State>
World::fetch_state(b2WorldId id)
{
	auto world = wrap(id);
	return world->state();
}

World::~World()
{
	b2DestroyWorld(_id);
	if (!mrb_nil_p(_custom_filter_block))
		state()->mrb()->gc_unregister(_custom_filter_block);
	if (!mrb_nil_p(_pre_solve_block))
		state()->mrb()->gc_unregister(_pre_solve_block);
}
bool
World::is_valid() const
{
	return b2World_IsValid(_id);
}
void
World::step(float dt, int substep_count)
{
	b2World_Step(_id, dt, substep_count);
}
std::vector<euler::physics::Body::MoveEvent>
World::body_events() const
{
	std::vector<Body::MoveEvent> events;
	b2BodyEvents b2_events = b2World_GetBodyEvents(_id);
	events.reserve(b2_events.moveCount);
	for (int i = 0; i < b2_events.moveCount; ++i) {
		const auto me = b2_events.moveEvents[i];
		const auto event = Body::MoveEvent::from_b2(me);
		events.push_back(event);
	}
	return events;
}
euler::physics::Shape::SensorEvents
World::sensor_events() const
{
	const auto events = b2World_GetSensorEvents(_id);
	return Shape::SensorEvents::from_b2(events);
}
euler::physics::Contact::Events
World::contact_events() const
{
	const auto events = b2World_GetContactEvents(_id);
	return Contact::Events::from_b2(events);
}
euler::physics::Joint::Events
World::joint_events() const
{
	const auto events = b2World_GetJointEvents(_id);
	return Joint::Events::from_b2(events);
}
b2TreeStats
World::overlap_aabb(const b2AABB &aabb, const b2QueryFilter filter,
    OverlapFn callback) const
{
	const auto ptr = static_cast<void *>(&callback);
	return b2World_OverlapAABB(_id, aabb, filter, overlap_result_fn, ptr);
}
b2TreeStats
World::overlap_aabb(const b2AABB &aabb, OverlapFn callback) const
{
	return overlap_aabb(aabb, b2DefaultQueryFilter(), std::move(callback));
}
b2TreeStats
World::overlap_shape(const b2ShapeProxy &proxy, const b2QueryFilter filter,
    OverlapFn callback) const
{
	const auto ptr = static_cast<void *>(&callback);
	return b2World_OverlapShape(_id, &proxy, filter, overlap_result_fn,
	    ptr);
}
b2TreeStats
World::overlap_shape(const b2ShapeProxy &proxy, OverlapFn callback) const
{
	return overlap_shape(proxy, b2DefaultQueryFilter(),
	    std::move(callback));
}
b2TreeStats
World::cast_ray(b2Vec2 point, b2Vec2 normal, b2QueryFilter filter,
    CastResultFn callback) const
{
	const auto ptr = static_cast<void *>(&callback);
	return b2World_CastRay(_id, point, normal, filter, cast_result_fn, ptr);
}
b2TreeStats
World::cast_ray(b2Vec2 point, b2Vec2 normal, CastResultFn callback) const
{
	return cast_ray(point, normal, b2DefaultQueryFilter(),
	    std::move(callback));
}
b2RayResult
World::cast_ray_closest(b2Vec2 origin, b2Vec2 translation, b2QueryFilter filter)
{
	return b2World_CastRayClosest(_id, origin, translation, filter);
}
b2TreeStats
World::cast_shape(const b2ShapeProxy &proxy, b2Vec2 translation,
    b2QueryFilter filter, CastResultFn callback) const
{
	const auto ptr = static_cast<void *>(&callback);
	return b2World_CastShape(_id, &proxy, translation, filter,
	    cast_result_fn, ptr);
}
b2TreeStats
World::cast_shape(const b2ShapeProxy &proxy, b2Vec2 translation,
    CastResultFn callback) const
{
	return cast_shape(proxy, translation, b2DefaultQueryFilter(),
	    std::move(callback));
}
float
World::cast_mover(const b2Capsule *mover, b2Vec2 translation,
    const b2QueryFilter filter) const
{
	return b2World_CastMover(_id, mover, translation, filter);
}
void
World::collide_mover(const b2Capsule *mover, const b2QueryFilter filter,
    PlaneResultFn callback) const
{
	const auto ptr = static_cast<void *>(&callback);
	b2World_CollideMover(_id, mover, filter, plane_result_fn, ptr);
}
void
World::collide_mover(const b2Capsule *mover, PlaneResultFn callback) const
{
	collide_mover(mover, b2DefaultQueryFilter(), std::move(callback));
}
void
World::enable_sleeping(bool enable)
{
	b2World_EnableSleeping(_id, enable);
}
bool
World::is_sleeping_enabled() const
{
	return b2World_IsSleepingEnabled(_id);
}
void
World::enable_continuous(bool enable)
{
	b2World_EnableContinuous(_id, enable);
}
bool
World::is_continuous_enabled() const
{
	return b2World_IsContinuousEnabled(_id);
}
void
World::set_restitution_threshold(float threshold)
{
	b2World_SetRestitutionThreshold(_id, threshold);
}
float
World::restitution_threshold()
{
	return b2World_GetRestitutionThreshold(_id);
}
void
World::set_hit_event_threshold(float threshold)
{
	b2World_SetHitEventThreshold(_id, threshold);
}
float
World::hit_event_threshold()
{
	return b2World_GetHitEventThreshold(_id);
}
void
World::set_custom_filter(CustomFilterFn callback)
{
	const auto ptr = static_cast<void *>(&callback);
	b2World_SetCustomFilterCallback(_id, custom_filter_fn, ptr);
}
void
World::set_pre_solve(PreSolveFn callback)
{
	const auto ptr = static_cast<void *>(&callback);
	b2World_SetPreSolveCallback(_id, pre_solve_fn, ptr);
}
void
World::set_gravity(const b2Vec2 &gravity)
{
	b2World_SetGravity(_id, gravity);
}
b2Vec2
World::gravity() const
{
	return b2World_GetGravity(_id);
}
void
World::explode(const b2ExplosionDef &def)
{
	b2World_Explode(_id, &def);
}
void
World::set_contact_tuning(float hertz, float damping_ratio, float push_speed)
{
	b2World_SetContactTuning(_id, hertz, damping_ratio, push_speed);
}
void
World::set_maximum_linear_speed(float speed)
{
	b2World_SetMaximumLinearSpeed(_id, speed);
}
float
World::maximum_linear_speed() const
{
	return b2World_GetMaximumLinearSpeed(_id);
}
int
World::awake_body_count() const
{
	return b2World_GetAwakeBodyCount(_id);
}
euler::util::Reference<euler::physics::Body>
World::create_body(const b2BodyDef &def) const
{
	auto id = b2CreateBody(_id, &def);
	return Body::wrap(id);
}

euler::util::Reference<euler::physics::Chain>
World::wrap_chain(const b2ChainId &id)
{
	const auto key = Chain::Key(id);
	if (_chains.contains(key)) { return _chains[key].strengthen(); }
	const auto ptr = new Chain(id);
	auto ref = util::Reference(ptr);
	_chains[key] = ref.weaken();
	return ref;
}

euler::util::Reference<euler::physics::Contact>
World::wrap_contact(const b2ContactId &id)
{
	const auto key = Contact::Key(id);
	if (_contacts.contains(key)) { return _contacts[key].strengthen(); }
	const auto ptr = new Contact(id);
	auto ref = util::Reference(ptr);
	_contacts[key] = ref.weaken();
	return ref;
}

void
World::drop_chain(const b2ChainId &id)
{
	_chains.erase(Chain::Key(id));
}

void
World::drop_contact(const b2ContactId &id)
{
	_contacts.erase(Contact::Key(id));
}

void
World::set_custom_filter(mrb_state *mrb, mrb_value block)
{
	const auto state = util::State::get(mrb);
	if (!mrb_nil_p(_custom_filter_block))
		state->rb().gc_unregister(_custom_filter_block);
	if (!mrb_nil_p(block)) state->rb().gc_register(block);
	_custom_filter_block = block;
}
void
World::set_pre_solve(mrb_state *mrb, mrb_value block)
{
	const auto state = util::State::get(mrb);
	if (!mrb_nil_p(_pre_solve_block))
		state->rb().gc_unregister(_pre_solve_block);
	if (!mrb_nil_p(block)) state->rb().gc_register(block);
	_pre_solve_block = block;
}
euler::util::Reference<euler::util::State>
World::state() const
{
	return _state.strengthen();
}