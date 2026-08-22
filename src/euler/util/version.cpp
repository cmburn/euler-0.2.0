/* SPDX-License-Identifier: ISC */

#include "euler/util/version.h"

#include <vector>

#ifdef EULER_NATIVE
#include <vulkan/vulkan_core.h>
#endif

#include "euler/util/ext.h"

using euler::util::Version;

static constexpr Version VERSION(0, 1, 0);

const mrb_data_type Version::TYPE
    = euler::util::pod_datatype<Version>("Euler::Util::Version");

static inline bool
in_bitmask(mrb_int n, size_t bits)
{
	const auto mask = (1ULL << bits) - 1;
	return (~mask & n) == 0;
}

static mrb_value
version_allocate(mrb_state *mrb, const mrb_value self)
{
	auto state = euler::util::State::get(mrb);
	auto obj = state->rb().data_object_alloc(mrb_class_ptr(self),
	    new Version(0, 1, 0), &Version::TYPE);
	return mrb_obj_value(obj);
}

static void
parse_version_string(mrb_state *mrb, const char *str, mrb_int &major,
    mrb_int &minor, mrb_int &patch)
{
	auto state = euler::util::State::get(mrb);
	if (*str == 'v') ++str;
	if (sscanf(str, "%ld.%ld.%ld", &major, &minor, &patch) != 3) {
		state->rb().raise(state->rb().argument_error(),
		    "Invalid version string format");
	}
}

static void
parse_single_version_arg(mrb_state *mrb, mrb_value arg, mrb_int &major,
    mrb_int &minor, mrb_int &patch)
{
	auto state = euler::util::State::get(mrb);
	if (mrb_string_p(arg)) {
		parse_version_string(mrb, state->rb().str_to_cstr(arg), major,
		    minor, patch);
		return;
	}
	if (mrb_integer_p(arg)) {
		const auto packed = static_cast<uint32_t>(mrb_integer(arg));
		major = (packed >> 22) & 0x3FF;
		minor = (packed >> 12) & 0x3FF;
		patch = packed & 0xFFF;
		return;
	}
	state->rb().raise(state->rb().argument_error(),
	    "Expected version string or packed integer for single argument");
}

static std::tuple<uint32_t, uint32_t, uint32_t>
read_version_args(mrb_state *mrb)
{
	auto state = euler::util::State::get(mrb);
	/*
	 * This can be passed one of the following ways:
	 * - No arguments: defaults to 0.1.0
	 * - Three integers: major, minor, patch
	 * - A version string: "v{major}.{minor}.{patch}"
	 * - A single integer: packed version
	 */
	mrb_int major = 0;
	mrb_int minor = 0;
	mrb_int patch = 0;

	switch (state->rb().get_argc()) {
	case 0:
		major = 0;
		minor = 1;
		patch = 0;
		break;
	case 1:
		parse_single_version_arg(mrb, state->rb().get_arg1(), major,
		    minor, patch);
		break;
	case 3: state->rb().get_args("|iii", &major, &minor, &patch); break;
	default:
		state->rb().raise(state->rb().argument_error(),
		    "Invalid number of arguments");
	}
	if (!in_bitmask(major, 10))
		state->rb().raise(state->rb().argument_error(),
		    "Major version out of range");
	if (!in_bitmask(minor, 10))
		state->rb().raise(state->rb().argument_error(),
		    "Minor version out of range");
	if (!in_bitmask(patch, 12))
		state->rb().raise(state->rb().argument_error(),
		    "Patch version out of range");
	return {
		static_cast<uint32_t>(major),
		static_cast<uint32_t>(minor),
		static_cast<uint32_t>(patch),
	};
}

static mrb_value
version_initialize(mrb_state *mrb, const mrb_value self_value)
{
	auto self = euler::util::unwrap_data<Version>(mrb, self_value,
	    &Version::TYPE);
	auto [major, minor, patch] = read_version_args(mrb);
	self->set_major(major);
	self->set_minor(minor);
	self->set_patch(patch);
	return mrb_nil_value();
}

static mrb_value
version_to_string(mrb_state *mrb, const mrb_value self_value)
{
	auto state = euler::util::State::get(mrb);
	const auto self = euler::util::unwrap_data<Version>(mrb, self_value,
	    &Version::TYPE);
	const auto str = self->to_string();
	return state->rb().str_new_cstr(str.c_str());
}

static mrb_value
version_to_int(mrb_state *mrb, const mrb_value self_value)
{
	const auto self = euler::util::unwrap_data<Version>(mrb, self_value,
	    &Version::TYPE);
	return mrb_fixnum_value(self->to_int());
}

static mrb_value
version_major(mrb_state *mrb, const mrb_value self_value)
{
	const auto self = euler::util::unwrap_data<Version>(mrb, self_value,
	    &Version::TYPE);
	return mrb_fixnum_value(self->major());
}

static mrb_value
version_minor(mrb_state *mrb, const mrb_value self_value)
{
	const auto self = euler::util::unwrap_data<Version>(mrb, self_value,
	    &Version::TYPE);
	return mrb_fixnum_value(self->minor());
}

static mrb_value
version_patch(mrb_state *mrb, const mrb_value self_value)
{
	const auto self = euler::util::unwrap_data<Version>(mrb, self_value,
	    &Version::TYPE);
	return mrb_fixnum_value(self->patch());
}

static mrb_value
version_set_major(mrb_state *mrb, const mrb_value self_value)
{
	auto state = euler::util::State::get(mrb);
	const auto self = euler::util::unwrap_data<Version>(mrb, self_value,
	    &Version::TYPE);
	mrb_int major;
	state->rb().get_args("i", &major);
	if (!in_bitmask(major, 10))
		state->rb().raise(state->rb().argument_error(),
		    "Major version out of range");
	self->set_major(static_cast<uint32_t>(major));
	return mrb_nil_value();
}

static mrb_value
version_set_minor(mrb_state *mrb, const mrb_value self_value)
{
	auto state = euler::util::State::get(mrb);
	const auto self = euler::util::unwrap_data<Version>(mrb, self_value,
	    &Version::TYPE);
	mrb_int minor;
	state->rb().get_args("i", &minor);
	if (!in_bitmask(minor, 10))
		state->rb().raise(state->rb().argument_error(),
		    "Minor version out of range");
	self->set_minor(static_cast<uint32_t>(minor));
	return mrb_nil_value();
}

static mrb_value
version_set_patch(mrb_state *mrb, const mrb_value self_value)
{
	auto state = euler::util::State::get(mrb);
	const auto self = euler::util::unwrap_data<Version>(mrb, self_value,
	    &Version::TYPE);
	mrb_int patch;
	state->rb().get_args("i", &patch);
	if (!in_bitmask(patch, 12))
		state->rb().raise(state->rb().argument_error(),
		    "Patch version out of range");
	self->set_patch(static_cast<uint32_t>(patch));
	return mrb_nil_value();
}

RClass *
Version::init(const Reference<State> &state, RClass *mod, RClass *)
{
	auto version = state->rb().define_class_under(mod, "Version",
	    state->object_class());
	MRB_SET_INSTANCE_TT(version, MRB_TT_DATA);
	state->modules().util.version = version;
	state->rb().define_class_method(version, "allocate", version_allocate,
	    MRB_ARGS_NONE());
	state->rb().define_method(version, "initialize", version_initialize,
	    MRB_ARGS_ANY());
	state->rb().define_method(version, "to_s", version_to_string,
	    MRB_ARGS_NONE());
	state->rb().define_method(version, "to_i", version_to_int,
	    MRB_ARGS_NONE());
	state->rb().define_method(version, "major", version_major,
	    MRB_ARGS_NONE());
	state->rb().define_method(version, "minor", version_minor,
	    MRB_ARGS_NONE());
	state->rb().define_method(version, "patch", version_patch,
	    MRB_ARGS_NONE());
	state->rb().define_method(version, "major=", version_set_major,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(version, "minor=", version_set_minor,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(version, "patch=", version_set_patch,
	    MRB_ARGS_REQ(1));
	return version;
}

uint32_t
Version::to_int() const
{
	return (static_cast<uint32_t>(_major) << 22U)
	    | (static_cast<uint32_t>(_minor) << 12U)
	    | static_cast<uint32_t>(_patch);
}

uint32_t
Version::to_vulkan() const
{
	return VK_MAKE_VERSION(_major, _minor, _patch);
}

Version
Version::parse(std::string_view str)
{
	const char *p = str.data();
	if (p[0] == 'v') ++p;
	uint32_t major = 0, minor = 0, patch = 0;
	std::vector<std::string_view> parts;
	for (const char *start = p; *p != '\0'; ++p) {
		if (p[0] != '.') continue;
		parts.emplace_back(start, p - start);
		start = p + 1;
	}
	switch (parts.size()) {
	case 3: patch = std::stoul(std::string(parts[2])); [[fallthrough]];
	case 2: minor = std::stoul(std::string(parts[1])); [[fallthrough]];
	case 1: major = std::stoul(std::string(parts[0])); break;
	default: throw std::invalid_argument("Invalid version string format");
	}
	return Version(major, minor, patch);
}

Version
euler::util::engine_version()
{
	return VERSION;
}
