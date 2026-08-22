/* SPDX-License-Identifier: ISC */

#ifndef EULER_UTIL_EXT_H
#define EULER_UTIL_EXT_H

#include <utility>

#include <mruby/throw.h>

#include "euler/util/error.h"
#include "euler/util/object.h"
#include "euler/util/state.h"

namespace euler::util {

namespace detail {
template <typename T> struct function_traits;

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...)> {
	using class_type = C;
	using return_type = R;
	using arg_types = std::tuple<Args...>;
	static constexpr std::size_t arity = sizeof...(Args);
};

template <typename R, typename C, typename... Args>
struct function_traits<R (C::*)(Args...) const> {
	using class_type = C;
	using return_type = R;
	using arg_types = std::tuple<Args...>;
	static constexpr std::size_t arity = sizeof...(Args);
};

template <typename R, typename... Args> struct function_traits<R (*)(Args...)> {
	using return_type = R;
	using arg_types = std::tuple<Args...>;
	static constexpr std::size_t arity = sizeof...(Args);
};

}

template <auto Method>
static constexpr mrb_func_t
wrap_method()
{
	using method_t = decltype(Method);
	using traits = detail::function_traits<method_t>;
	using SelfT = traits::class_type;
	using ReturnT = traits::return_type;

	static_assert(traits::arity == 1,
	    "method must take exactly one argument");
	using Arg0 = std::tuple_element_t<0, typename traits::arg_types>;
	static_assert(std::is_same_v<ReturnT, mrb_value>,
	    "method must return mrb_value");
	static_assert(std::is_same_v<Arg0, const Reference<State> &>,
	    "method must accept const Reference<State>&");

	/* ReSharper disable once CppParameterMayBeConstPtrOrRef */
	return [](mrb_state *mrb, mrb_value self) -> mrb_value {
		mrb_value exc = mrb_nil_value();
		try {
			const auto state = State::get(mrb);
			auto self_obj = Reference<SelfT>::unwrap(self);
			if (self_obj == nullptr)
				throw ArgumentError(state, "Passed null self");
			return (self_obj.get()->*Method)(state);
		} catch (std::exception &e) {
			auto state = State::get(mrb);
			// exc = wrap_exception(State::get(mrb), e);
		}
		/* can't have any uncalled destructors, which gets a bit
		 * tricky if mruby isn't built with support for c++ exceptions.
		 * This gets even trickier when we're operating with an mruby
		 * state that's wrapped in our virtual class. This technically
		 * could segfault, if the RubyState is destroyed after we
		 * fetch it but before we call mrb_raise, but that's unlikely
		 * enough to worry about for now, as the containing State should
		 * have the same lifetime as the passed mrb_state. */
		State::get(mrb)->rb().exc_raise(exc);
	};
}

template <auto function>
static constexpr mrb_func_t
wrap_function()
{
	using function_t = decltype(function);
	using traits = detail::function_traits<function_t>;
	static_assert(traits::arity == 1,
	    "function must take exactly one argument");
	using Arg0 = std::tuple_element_t<0, typename traits::arg_types>;
	static_assert(std::is_same_v<typename traits::return_type, mrb_value>,
	    "function must return mrb_value");
	static_assert(std::is_same_v<Arg0, const Reference<State> &>,
	    "function must accept const Reference<State>&");

	/* ReSharper disable once CppParameterMayBeConstPtrOrRef */
	return [](mrb_state *mrb, mrb_value /* self */) -> mrb_value {
		const auto state = State::get(mrb);
		return function(state);
	};
}

template <auto method>
void
define_method(const Reference<State> &state, RClass *cls, const char *name,
    const mrb_aspec args = MRB_ARGS_NONE())
{
	state->rb().define_method(cls, name, wrap_method<method>(), args);
}

template <auto function>
void
define_function(const Reference<State> &state, RClass *cls, const char *name,
    const mrb_aspec args = MRB_ARGS_NONE())
{
	static constexpr auto fn = wrap_function<function>();
	state->rb().define_class_method(cls, name, fn, args);
}

RClass *init(const Reference<State> &state, RClass *mod, RClass * = nullptr);

template <typename T>
static void
dfree(mrb_state *, void *ptr)
{
	if (ptr == nullptr) return;
	auto ref = Reference<T>::unwrap(ptr);
	ref.decrement();
}

template <typename T>
static Reference<T>
unwrap(const Reference<State> &state, mrb_value self_value,
    const mrb_data_type *type)
{
	const void *ptr = state->rb().data_check_get_ptr(self_value, type);
	return Reference<T>::unwrap(ptr);
}

template <typename T>
static Reference<T>
unwrap(const Reference<State> &state, mrb_value self_value)
{
	const void *ptr
	    = state->rb().data_check_get_ptr(self_value, &T::TYPE);
	return Reference<T>::unwrap(ptr);
}

template <typename T>
static constexpr mrb_data_type
datatype(const char *name)
{
	return mrb_data_type {
		.struct_name = name,
		.dfree = util::dfree<T>,
	};
}

template <typename T>
static constexpr mrb_data_type
pod_datatype(const char *name)
{
	return mrb_data_type {
		.struct_name = name,
		.dfree =
		    [](mrb_state *, void *ptr) {
		            auto self = static_cast<T *>(ptr);
		            self->~T();
		    },
	};
}

template <typename T>
static T *
unwrap_data(mrb_state *mrb, const mrb_value &value, const mrb_data_type *type)
{
	auto state = State::get(mrb);
	if (mrb_nil_p(value)) {
		state->rb().raisef(state->rb().type_error(),
		    "Expected a %s object", type->struct_name);
	}
	if (auto ptr = state->rb().data_get_ptr(value, type); ptr != nullptr)
		return static_cast<T *>(ptr);
	state->rb().raisef(state->rb().type_error(), "Expected a %s object",
	    type->struct_name);
	std::unreachable();
}

#define BIND_MRUBY(NAME, TYPENAME, MODULE)                                     \
public:                                                                        \
	static constexpr auto TYPE = ::euler::util::datatype<TYPENAME>(NAME);  \
	static RClass *init(                                                   \
	    const ::euler::util::Reference<::euler::util::State> &state,       \
	    RClass *mod, RClass *super = nullptr);                             \
	static RClass *fetch_class(                                            \
	    const ::euler::util::Reference<::euler::util::State> &state)       \
	{                                                                      \
		return state->modules().MODULE;                                \
	}                                                                      \
                                                                               \
private:                                                                       \
	static_assert(true)

#define BIND_MRUBY_DATA(NAME, TYPENAME, MODULE)                                \
public:                                                                        \
	static const mrb_data_type TYPE;                                       \
	static RClass *init(                                                   \
	    const ::euler::util::Reference<::euler::util::State> &state,       \
	    RClass *mod, RClass *super = nullptr);                             \
	static RClass *fetch_class(                                            \
	    const ::euler::util::Reference<::euler::util::State> &state)       \
	{                                                                      \
		return state->modules().MODULE;                                \
	}                                                                      \
                                                                               \
private:                                                                       \
	static_assert(true)

float read_hash_float(mrb_state *mrb, const mrb_value hash, const mrb_sym key,
    const float default_value = 0.0f);

} /* namespace euler::util */

#endif /* EULER_UTIL_EXT_H */
