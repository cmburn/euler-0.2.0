/* SPDX-License-Identifier: ISC */

#ifndef EULER_GRAPHICS_MATH_H
#define EULER_GRAPHICS_MATH_H

#include <array>
#include <complex>

#include <glm/glm.hpp>

#include "euler/util/object.h"
#include "euler/util/ruby_state.h"
#include "euler/util/state.h"

namespace euler::graphics {

template <typename T>
static inline float
angle(glm::vec<2, T> v)
{
	return std::atan2f(v.y(), v.x());
}

template <typename T>
static inline float
length(glm::vec<2, T> v)
{
	return std::sqrtf(v.x() * v.x() + v.y() * v.y());
}

static inline float
deg2rad(const float deg)
{
	static constexpr float factor = (M_PI / 180.f);
	return deg * factor;
}

static inline float
rad2deg(const float rad)
{
	static constexpr float factor = (M_PI / 180.f);
	return rad * factor;
}

template <typename T>
static inline glm::vec<2, T>
deg2vec(const float deg)
{
	const auto rad = deg2rad(deg);
	return glm::vec<2, T> { std::cosf(rad), std::sinf(rad) };
}

template <typename T>
static constexpr bool
is_supported_numeric()
{
	if constexpr (std::is_same_v<T, int16_t>) return true;
	if constexpr (std::is_same_v<T, int32_t>) return true;
	if constexpr (std::is_same_v<T, int64_t>) return true;
	if constexpr (std::is_same_v<T, uint16_t>) return true;
	if constexpr (std::is_same_v<T, uint32_t>) return true;
	if constexpr (std::is_same_v<T, uint64_t>) return true;
	if constexpr (std::is_same_v<T, float>) return true;
	if constexpr (std::is_same_v<T, double>) return true;
	if constexpr (std::is_same_v<T, std::complex<float>>) return true;
	if constexpr (std::is_same_v<T, std::complex<double>>) return true;
	return false;
}

template <typename T>
static constexpr bool
is_complex()
{
	if constexpr (std::is_same_v<T, std::complex<float>>) return true;
	if constexpr (std::is_same_v<T, std::complex<double>>) return true;
	return false;
}

template <typename T>
static constexpr bool
can_decompose()
{
	if constexpr (std::is_same_v<T, float>) return true;
	if constexpr (std::is_same_v<T, double>) return true;
	if constexpr (std::is_same_v<T, std::complex<float>>) return true;
	if constexpr (std::is_same_v<T, std::complex<double>>) return true;
	return false;
}

/* ReSharper disable once CppDFAUnreachableFunctionCall */
template <typename T>
static std::complex<T>
unwrap_complex(const util::Reference<util::RubyState> &mrb, mrb_value arg)
{
	const auto real_value = mrb->funcall(arg, "real", 0);
	const auto imag_value = mrb->funcall(arg, "imag", 0);
	const auto real = static_cast<T>(mrb->to_flo(real_value));
	const auto imag = static_cast<T>(mrb->to_flo(imag_value));
	return std::complex<T>(real, imag);
}

template <typename T>
static mrb_value
wrap_complex(const util::Reference<util::RubyState> &mrb,
    const std::complex<T> &v)
{
	const auto real = mrb->float_value(static_cast<mrb_float>(v.real()));
	const auto imag = mrb->float_value(static_cast<mrb_float>(v.imag()));
	const auto cls = mrb->class_get("Complex");
	const std::array args = { real, imag };
	return mrb->obj_new(cls, args.size(), args.data());
}

template <typename T> struct numeric_info : std::false_type {
	static_assert(is_supported_numeric<T>());
};

template <> struct numeric_info<int16_t> {
	typedef mrb_int mrb_type;
	static constexpr auto abbreviation = "I16";
	static constexpr auto can_decompose = false;
	static constexpr bool is_complex = false;
};

template <> struct numeric_info<int32_t> {
	typedef mrb_int mrb_type;
	static constexpr auto abbreviation = "I32";
	static constexpr auto can_decompose = false;
	static constexpr bool is_complex = false;
};

template <> struct numeric_info<int64_t> {
	typedef mrb_int mrb_type;
	static constexpr auto abbreviation = "I64";
	static constexpr auto can_decompose = false;
	static constexpr bool is_complex = false;
};

template <> struct numeric_info<uint16_t> {
	typedef mrb_uint mrb_type;
	static constexpr auto abbreviation = "U16";
	static constexpr auto can_decompose = false;
	static constexpr bool is_complex = false;
};

template <> struct numeric_info<uint32_t> {
	typedef mrb_uint mrb_type;
	static constexpr auto abbreviation = "U32";
	static constexpr auto can_decompose = false;
	static constexpr bool is_complex = false;
};

template <> struct numeric_info<uint64_t> {
	typedef mrb_uint mrb_type;
	static constexpr auto abbreviation = "U64";
	static constexpr auto can_decompose = false;
	static constexpr bool is_complex = false;
};

template <> struct numeric_info<float> {
	typedef mrb_float mrb_type;
	static constexpr auto abbreviation = "F32";
	static constexpr auto can_decompose = true;
	static constexpr bool is_complex = false;
};

template <> struct numeric_info<double> {
	typedef mrb_float mrb_type;
	static constexpr auto abbreviation = "F64";
	static constexpr auto can_decompose = true;
};

template <> struct numeric_info<std::complex<float>> {
	typedef mrb_value mrb_type;
	static constexpr auto abbreviation = "C32";
	static constexpr auto can_decompose = true;
};

template <> struct numeric_info<std::complex<double>> {
	typedef mrb_value mrb_type;
	static constexpr auto abbreviation = "C64";
	static constexpr auto can_decompose = true;
};

template <typename T>
static T
unwrap_num(const util::Reference<util::State> &state, mrb_value arg)
{
	static_assert(is_supported_numeric<T>());
	/* ReSharper disable CppRedundantCastExpression */
	if constexpr (std::is_same_v<T, int16_t>) {
		return static_cast<int16_t>(
		    mrb_integer(state->mrb()->to_int(arg)));
	}
	if constexpr (std::is_same_v<T, int32_t>) {
		return static_cast<int32_t>(
		    mrb_integer(state->mrb()->to_int(arg)));
	}
	if constexpr (std::is_same_v<T, int64_t>) {
		return static_cast<int64_t>(
		    mrb_integer(state->mrb()->to_int(arg)));
	}
	if constexpr (std::is_same_v<T, uint16_t>) {
		return static_cast<uint16_t>(
		    mrb_integer(state->mrb()->to_int(arg)));
	}
	if constexpr (std::is_same_v<T, uint32_t>) {
		return static_cast<uint32_t>(
		    mrb_integer(state->mrb()->to_int(arg)));
	}
	if constexpr (std::is_same_v<T, float>) {
		return static_cast<float>(state->mrb()->to_flo(arg));
	}
	if constexpr (std::is_same_v<T, double>) {
		return static_cast<double>(state->mrb()->to_flo(arg));
	}
	if constexpr (std::is_same_v<T, std::complex<float>>) {
		return unwrap_complex<float>(state->mrb(), arg);
	}
	if constexpr (std::is_same_v<T, std::complex<double>>) {
		return unwrap_complex<double>(state->mrb(), arg);
	}
	/* ReSharper restore CppRedundantCastExpression */
	return {};
}

template <typename T>
mrb_value
wrap_num(const util::Reference<util::RubyState> &mrb, T v)
{
	using value_type = std::remove_cv_t<std::remove_reference_t<T>>;
	static_assert(is_supported_numeric<value_type>());
	if constexpr (std::is_same_v<value_type, int16_t>)
		return mrb->int_value(static_cast<mrb_int>(v));
	if constexpr (std::is_same_v<value_type, int32_t>)
		return mrb->int_value(static_cast<mrb_int>(v));
	if constexpr (std::is_same_v<value_type, int64_t>)
		return mrb->int_value(static_cast<mrb_int>(v));
	if constexpr (std::is_same_v<value_type, uint16_t>)
		return mrb->int_value(static_cast<mrb_int>(v));
	if constexpr (std::is_same_v<value_type, uint32_t>)
		return mrb->int_value(static_cast<mrb_int>(v));
	if constexpr (std::is_same_v<value_type, uint64_t>)
		return mrb->int_value(static_cast<mrb_int>(v));
	if constexpr (std::is_same_v<value_type, float>)
		return mrb->float_value(static_cast<mrb_float>(v));
	if constexpr (std::is_same_v<value_type, double>)
		return mrb->float_value(static_cast<mrb_float>(v));
	if constexpr (std::is_same_v<value_type, std::complex<float>>)
		return wrap_complex<float>(mrb, v);
	if constexpr (std::is_same_v<value_type, std::complex<double>>)
		return wrap_complex<double>(mrb, v);
	return mrb_nil_value();
}

namespace detail {
template <typename T> struct pod_type {
	static_assert(graphics::is_supported_numeric<T>());
	typedef T type;
};
template <> struct pod_type<std::complex<float>> {
	typedef float type;
};
template <> struct pod_type<std::complex<double>> {
	typedef double type;
};

template <typename T> using pod_type_t = pod_type<T>::type;
}

template <typename To, typename From>
To
cast_numeric(From value)
{
	static_assert(is_supported_numeric<To>());
	static_assert(is_supported_numeric<From>());
	if constexpr (std::is_same_v<To, From>) {
		return value;
	} else if constexpr (std::is_integral_v<To>
	    && std::is_integral_v<From>) {
		return static_cast<To>(value);
	} else if constexpr (std::is_floating_point_v<To>
	    && std::is_integral_v<From>) {
		return static_cast<To>(value);
	} else if constexpr (std::is_integral_v<To>
	    && std::is_floating_point_v<From>) {
		return static_cast<To>(std::round(value));
	} else if constexpr (std::is_floating_point_v<To>
	    && std::is_floating_point_v<From>) {
		return static_cast<To>(value);
	} else if constexpr (is_complex<To>()
	    && std::is_floating_point_v<From>) {
		return To(static_cast<detail::pod_type_t<To>>(value), 0);
	} else if constexpr (is_complex<To>() && is_complex<From>()) {
		auto real = static_cast<detail::pod_type_t<To>>(value.real());
		auto imag = static_cast<detail::pod_type_t<To>>(value.imag());
		return To(real, imag);
	} else {
		static_assert(!sizeof(To *), "Unsupported numeric cast");
	}
	return To {};
}

inline bool
is_complex(const mrb_state *mrb, const mrb_value v)
{
	const auto state = util::State::get(mrb);
	const auto complex_class = state->mrb()->class_get("Complex");
	return state->mrb()->obj_is_instance_of(v, complex_class);
}

inline bool
is_numeric(const mrb_state *mrb, const mrb_value v)
{
	return is_complex(mrb, v) || mrb_float_p(v) || mrb_fixnum_p(v);
}

} /* namespace euler::graphics */

#endif /* EULER_GRAPHICS_MATH_H */
